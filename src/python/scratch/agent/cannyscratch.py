#!/usr/bin/python
#
# Cannybots Scratch integration
# By Wayne Keenan 02/12/2014, rewritten 27/05//015
# www.cannybots.com 

from cannybots.scratchagent import ScratchAgent

if __name__ == "__main__":
    agent = ScratchAgent()
    agent.start()  # will block until error (e.g. connection loss)
    agent.stop()
