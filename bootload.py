#!/usr/bin/env python

import serial

bootloader_error_codes = {
  '0' : "OK",
  '1' : "Intel HEX Invalid",
  '2' : "Bad Checksum",
  '3' : "Bad Address",
  '4' : "Bad Record Type",
  '5' : "Record Too Long"
}

def download_code(ihx_file, serial_port):
  for line in ihx_file.readlines():
    print line[:-1],
    serial_port.write(line)
    rc = serial_port.read()
    print " RC =", rc,
    if rc in bootloader_error_codes:
      print "(%s)" % bootloader_error_codes[rc]
    else:
      print "(Unknown Error)"
    if (rc != '0'):
      print "Error Downloading Code!"
      return False
  return True

if __name__ == '__main__':
  import sys
  if (len(sys.argv) != 3):
    print """
    CC Bootloader Download Utility
    
    Usage:  ./bootload.py serial_port hex_file
    """
    sys.exit(1)
  
  serial_port_name = sys.argv[1]
  ihx_filename = sys.argv[2]

  ihx_file = open(ihx_filename, 'r')
  serial_port = serial.Serial(serial_port_name)
  download_code(ihx_file, serial_port)


