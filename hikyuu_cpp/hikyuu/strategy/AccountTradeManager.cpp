/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-23
 *     Author: fasiondog
 */

#include <nlohmann/json.hpp>
#include "../utilities/arithmetic.h"
#include "AccountTradeManager.h"

using nlohmann::json;

namespace hku {

#define HKU_SERVER_URL "http://localhost:9001"
#define HKU_SERVER_LOGIN_API "/hku/account/v1/login"

#define HTTP_STATUS_OK 200
#define HTTP_STATUS_BAD_REQUEST 400

static string getHttpClientErrorMsg(httplib::Error err) {
    string result;
    if (httplib::Error::Success == err) {
        result = "Success";
    } else if (httplib::Error::Unknown == err) {
        result = "Unknonw error";
    } else if (httplib::Error::Connection == err) {
        result = "Connection error";
    } else if (httplib::Error::BindIPAddress == err) {
        result = "BindIPAddress error";
    } else if (httplib::Error::Read == err) {
        result = "Read error";
    } else if (httplib::Error::Write == err) {
        result = "Write error";
    } else if (httplib::Error::ExceedRedirectCount == err) {
        result = "ExceedRedirectCount error";
    } else if (httplib::Error::Canceled == err) {
        result = "Canceled error";
    } else if (httplib::Error::SSLConnection == err) {
        result = "SSLConnection error";
    } else if (httplib::Error::SSLLoadingCerts == err) {
        result = "SSLLoadingCerts error";
    } else if (httplib::Error::SSLServerVerification == err) {
        result = "SSLServerVerification error";
    } else if (httplib::Error::UnsupportedMultipartBoundaryChars == err) {
        result = "UnsupportedMultipartBoundaryChars error";
    } else if (httplib::Error::Compression == err) {
        result = "Compression error";
    } else {
        result = "Other unknonw error";
    }
    return result;
}

AccountTradeManager::AccountTradeManager(const string& name, const string& pwd)
: TradeManagerBase(name, TC_Zero()), m_user(name), m_password(pwd) {
    m_client = std::make_unique<httplib::Client>(HKU_SERVER_URL);
    trim(m_user);
    if (m_user.empty()) {
        HKU_ERROR("User name is empty.");
        return;
    }

    string req(fmt::format(R"({{"user":"{}", "password":"{}"}})", name, pwd));
    auto res = m_client->Post(HKU_SERVER_LOGIN_API, req.c_str(), req.size(), "application/json");
    if (!res) {
        HKU_ERROR("http client err: {}", getHttpClientErrorMsg(res.error()));
        return;
    }

    if (HTTP_STATUS_OK == res->status) {
        auto data = json::parse(res->body);
        m_token = data["hku_token"].get<string>();
    } else if (HTTP_STATUS_BAD_REQUEST == res->status) {
        HKU_ERROR("Bad request: {}", res->body);
    } else {
        HKU_ERROR("http response status: {}", res->status);
    }
}

}  // namespace hku