#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import pickle

TOPIC_LEN = 16

def string_to_topic(string):
    """
    Returns the topic from the string.

    string -- string to convert
    """

    # truncuate/pad to 16 characters
    topic = string.ljust(TOPIC_LEN, '\x00')[:TOPIC_LEN]
    return topic.encode('utf-8')


def pack(data, topic=""):
    """
    Returns a packet for the given arguments.

    data -- data to pack
    topic -- topic to pack
    """
    data = pickle.dumps(data)
    topic = string_to_topic(topic)
    return topic + data

def unpack(packet):
    """
    Unpack a packet and returns (topic, data) as a tuple.

    packet -- Packet to unpack
    """
    topic, data = packet[:TOPIC_LEN], packet[TOPIC_LEN:]
    data = pickle.loads(data)
    return topic, data
