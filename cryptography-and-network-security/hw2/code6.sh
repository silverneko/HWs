#!/usr/bin/env bash

IP_RANGE="28 29 30 31 90 91 172"

for ip_sec in $IP_RANGE; do
  ip_prefix="140.112.$ip_sec.1/24"
  ips=`nmap -p 443 $ip_prefix -oG - | awk '/Up$/{print $2}'`
  for ip in $ips; do
    host_port="$ip:443"
    echo "Connecting to $host_port"
    cert=`timeout 1 openssl s_client -connect $host_port -showcerts </dev/null 2>/dev/null`
    if [[ $cert =~ "the-real-reason-to-not-use-sigkill" ]]; then
      echo "Found target certificate from ip: $ip"
      echo "$cert"
      exit
    fi
  done
done
