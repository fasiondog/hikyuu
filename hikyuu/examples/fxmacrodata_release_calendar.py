"""Fetch FXMacroData macro release dates for strategy filters.

Use this alongside Hikyuu strategy logic when a backtest should avoid opening
fresh risk around top-tier macro announcements.
"""

import json
from datetime import date, timedelta
from urllib.parse import urlencode
from urllib.request import urlopen


BASE_URL = "https://fxmacrodata.com/api/v1/calendar/{currency}"


def fetch_calendar(currency="USD", start_date=None, end_date=None, timeout=20):
    today = date.today()
    start_date = start_date or today.isoformat()
    end_date = end_date or (today + timedelta(days=14)).isoformat()
    query = urlencode({"start_date": start_date, "end_date": end_date})

    with urlopen(f"{BASE_URL.format(currency=currency)}?{query}", timeout=timeout) as response:
        payload = json.load(response)

    return payload.get("data", [])


def top_tier_blackout_dates(events):
    blackout_dates = {
        (event.get("announcement_datetime_utc") or event.get("announcement_datetime_local") or event.get("date", ""))[:10]
        for event in events
        if event.get("top_tier_for_currency") or event.get("market_tier") == 1
    }
    return sorted(date_value for date_value in blackout_dates if date_value)


if __name__ == "__main__":
    events = fetch_calendar(start_date="2026-07-01", end_date="2026-07-20")
    print(top_tier_blackout_dates(events))
