void SysProvEvent(arduino_event_t *sys_event) { // this is mostly error handling and reporting
  switch (sys_event->event_id) {

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      debug("Connected with IP address : ");
      debugln(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
      netStatus(GREEN);
      break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      debugln("Starting WiFi connection");
      netStatus(RED);
      break;

    case ARDUINO_EVENT_PROV_START:
      debugln("Provisioning started\nConnect to the device WiFi now");
      netStatus(RED);
      break;

    case ARDUINO_EVENT_PROV_CRED_RECV:
      {
        debugln("Credentials Received");
        debug("\tSSID : ");
        debugln((const char *)sys_event->event_info.prov_cred_recv.ssid);
        debug("\tPassword : ");
        debugln((char const *)sys_event->event_info.prov_cred_recv.password);
        break;
      }

    case ARDUINO_EVENT_PROV_CRED_FAIL:
      {
        netStatus(RED);
        debugln("I'm sorry but that didn't work.  Reset and try again");
        if (sys_event->event_info.prov_fail_reason == WIFI_PROV_STA_AUTH_ERROR)
          debugln("Incorrect Password");
        else
          break;
      }

    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
      debugln("Provisioning Successful");
      netStatus(GREEN);
      break;

    case ARDUINO_EVENT_PROV_END:
      debugln("Provisioning Complete");
      netStatus(GREEN);
      break;

    default:
      break;
  }
}