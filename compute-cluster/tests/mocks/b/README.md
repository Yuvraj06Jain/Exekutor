# Mock B: fake worker and fake client (B writes these)
Fake worker: REGISTER, heartbeat every 2 s, then stop heartbeating to test SUSPECTED/DEAD.
Fake client: CLIENT_LOGIN, SUBMIT jobs, STATUS_REQ. Lets B test the scheduler alone.
