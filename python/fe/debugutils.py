#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Module: debugutils.py
Author: Ursus Wigger <wigger.ursus@gmail.com>
Description:
This module contains a simple debug interface between a server and a client.
The server contains all the messages while the clients can connect to the server
to receive debugging information.
"""

from client import DebugClient
from server import DebugServer
