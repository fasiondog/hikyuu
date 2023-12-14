/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-28
 *     Author: fasiondog
 */

#include "common/base64.h"
#include "user/model/TokenModel.h"
#include "TokenCache.h"
#include "RestErrorCode.h"
#include "RestHandle.h"
#include "filter.h"

namespace hku {

std::string createToken(uint64_t userid) {
    Datetime now = Datetime::now();
    srand((unsigned)now.minute());
    int k = rand() % 1000;
    return base64_encode(fmt::format("{} {} {}", userid, now, k), false);
}

void AuthorizeFilter(HttpHandle *handle) {
    std::string token = handle->getReqHeader("hku_token");
    AUTHORIZE_CHECK(!token.empty(), AuthorizeErrorCode::MISS_TOKEN,
                    handle->_ctr("authorize", "Miss token"));

    std::string errmsg = handle->_ctr("authorize", "Invalid token");
    if (!TokenCache::have(token)) {
        TokenModel token_record;
        DB::getConnect()->load(token_record, fmt::format(R"(token="{}")", token));
        AUTHORIZE_CHECK(token_record.id() != 0, AuthorizeErrorCode::FAILED_AUTHORIZED, errmsg);
        TokenCache::put(token);
    }

    try {
        std::string decode_token = base64_decode(token);
        auto pos = decode_token.find_first_of(" ");
        AUTHORIZE_CHECK(pos != std::string::npos, AuthorizeErrorCode::FAILED_AUTHORIZED, errmsg);

        auto userid_str = decode_token.substr(0, pos);
        uint64_t userid = 0;
        Datetime create_time;

        userid = std::stoull(userid_str);
        auto d_pos = decode_token.find_last_of(" ");
        AUTHORIZE_CHECK(d_pos != std::string::npos, AuthorizeErrorCode::FAILED_AUTHORIZED, errmsg);
        create_time = Datetime(decode_token.substr(pos + 1, d_pos - pos));

        Datetime expired_time = create_time + TimeDelta(30);
        Datetime now = Datetime::now();
        if (now > expired_time) {
            TokenCache::remove(token);
            {
                TokenModel token_record;
                auto con = DB::getConnect();
                AutoTransAction trans(con);
                con->load(token_record, fmt::format(R"(token="{}")", token));
                con->exec(fmt::format(R"(delete from {} where token="{}")",
                                      TokenModel::getTableName(), token));
            }
            throw HttpUnauthorizedError(AuthorizeErrorCode::AUTHORIZE_EXPIRED,
                                        handle->_ctr("authorize", "token is expired"));
        }

        RestHandle *rest_handle = dynamic_cast<RestHandle *>(handle);
        rest_handle->setCurrentUserId(userid);

        if (expired_time - now <= TimeDelta(3)) {
            std::string new_token = createToken(userid);
            TokenModel token_record;
            token_record.setToken(new_token);
            DB::getConnect()->save(token_record);
            TokenCache::put(new_token);
            rest_handle->setUpdateToken(new_token);
        }

    } catch (const HttpUnauthorizedError &) {
        throw;

    } catch (std::exception &e) {
        APP_ERROR(e.what());
        throw HttpUnauthorizedError(AuthorizeErrorCode::FAILED_AUTHORIZED, errmsg);
    }
}

}  // namespace hku