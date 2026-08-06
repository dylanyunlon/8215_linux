#!/bin/sh
dbus-launch --auto-syntax --exit-with-session >>/etc/profile &
##echo "d-bus per-session daemon address is: $dbus_session_bus_address &"