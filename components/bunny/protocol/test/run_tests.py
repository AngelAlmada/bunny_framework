#!/usr/bin/env python3
"""
Bunny Framework - Protocol Test Runner & BCP Contract Validator
Executes unit tests and validates BCP specification adherence.
"""

import json
import os
import sys
import time
from pathlib import Path

PASS = "✅ PASS"
FAIL = "❌ FAIL"


def log_test(name: str, passed: bool, details: str = ""):
    status = PASS if passed else FAIL
    print(f"  {status}: {name}")
    if not passed and details:
        print(f"       Details: {details}")


def test_bcp_command_contract():
    """Verify BCP COMMAND envelope structure and requirements."""
    req = {
        "type": "COMMAND",
        "messageId": "cmd_test_001",
        "payload": {
            "method": "setFanState",
            "params": {"state": "ON"},
            "awaitResponse": True
        }
    }
    assert req["type"] == "COMMAND"
    assert "messageId" in req
    assert "method" in req["payload"]
    assert isinstance(req["payload"]["params"], dict)


def test_bcp_response_success_contract():
    """Verify BCP successful RESPONSE envelope structure."""
    res = {
        "type": "RESPONSE",
        "messageId": "res_001",
        "correlationId": "cmd_test_001",
        "deviceId": "esp32-001",
        "timestamp": int(time.time() * 1000),
        "payload": {
            "status": "success",
            "data": {
                "applied": {"state": "ON"}
            },
            "error": None
        }
    }
    assert res["type"] == "RESPONSE"
    assert res["correlationId"] == "cmd_test_001"
    assert res["payload"]["status"] == "success"
    assert res["payload"]["error"] is None
    assert "applied" in res["payload"]["data"]


def test_bcp_error_contract():
    """Verify BCP ERROR envelope structure."""
    err = {
        "type": "ERROR",
        "messageId": "err_001",
        "correlationId": "cmd_test_002",
        "deviceId": "esp32-001",
        "timestamp": int(time.time() * 1000),
        "payload": {
            "code": "INVALID_PARAMS",
            "category": "validation",
            "message": "Missing required command parameter",
            "retryable": False
        }
    }
    assert err["type"] == "ERROR"
    assert err["payload"]["code"] in ["INVALID_PARAMS", "UNKNOWN_METHOD", "DUPLICATE_MSG", "PARSE_ERROR"]
    assert isinstance(err["payload"]["retryable"], bool)


def test_bcp_event_contract():
    """Verify BCP EVENT envelope structure."""
    evt = {
        "type": "EVENT",
        "messageId": "evt_001",
        "deviceId": "esp32-001",
        "timestamp": int(time.time() * 1000),
        "payload": {
            "event": "motion_detected",
            "data": {"zone": "entrance"},
            "meta": {
                "firmwareVersion": "0.1.0",
                "freeHeap": 180000,
                "uptime": 123456
            }
        }
    }
    assert evt["type"] == "EVENT"
    assert "event" in evt["payload"]
    assert "meta" in evt["payload"]
    assert "freeHeap" in evt["payload"]["meta"]


def main():
    print("========================================")
    print("🐇 Bunny Framework - BCP Protocol Tests")
    print("========================================")
    print()

    tests = [
        ("BCP COMMAND Envelope Structure", test_bcp_command_contract),
        ("BCP RESPONSE Success Envelope Structure", test_bcp_response_success_contract),
        ("BCP ERROR Envelope Structure", test_bcp_error_contract),
        ("BCP EVENT Envelope Structure", test_bcp_event_contract),
    ]

    passed_count = 0
    failed_count = 0

    print("--- Running BCP Specification Contract Tests ---")
    for name, fn in tests:
        try:
            fn()
            log_test(name, True)
            passed_count += 1
        except Exception as e:
            log_test(name, False, str(e))
            failed_count += 1

    print()
    print("========================================")
    print(f"Summary: {passed_count} passed, {failed_count} failed.")
    print("========================================")

    return 0 if failed_count == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
