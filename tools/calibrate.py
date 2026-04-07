#!/usr/bin/env python3
"""
OpenTPMS Factory Calibration Tool

Connects via BLE, reads sensor at known pressures,
computes polynomial correction, writes to sensor flash.

Requires: bleak, numpy
TODO: See docs/implementation-plan.md Task 15
"""
