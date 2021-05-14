#!/usr/bin/python

# v1.01

import os
import tempfile
import cv2
import numpy
import datetime
import subprocess

camIP = '192.168.1.66'    # Cam1


def encode_hour( f_year, f_day, f_hour ):

  cmd_call = 'wget --quiet --recursive --no-directories --no-host-directories --timestamping --level=1 --accept="PIC*jpg" "http://{}/ai-cam/{}/{}"'.format( camIP, f_day, f_hour )
  # subprocess.call( cmd_call, shell=True )
  print( cmd_call )

  file_list = ''
  aicam_dir = os.getcwd()

  sorted_ls = sorted( os.listdir( aicam_dir ) )
  for aicam_fn in sorted_ls:

    if aicam_fn.startswith( 'PIC-{}{}{}'.format( f_year, f_day, f_hour ) ) and aicam_fn.endswith( 'jpg' ):

      aicam_image = cv2.imread( os.path.join( aicam_dir, aicam_fn ) )
      aicam_average = numpy.average( aicam_image )

      if aicam_average > 5:
        file_list = file_list + aicam_fn + ' '

      continue

  if file_list != '':

    cmd_call = 'cat {} | ffmpeg -f image2pipe -i - cam1-{}{}.mkv'.format( file_list, f_day, f_hour )
    # subprocess.call( cmd_call, shell=True )
    print( cmd_call )



def fetch_whole_day( f_year, f_day ):

  for i_hour in range( 24 ):
    f_hour = '{:02}'.format( i_hour )
    encode_hour( f_year, f_day, f_hour )



### Main loop

time_now = datetime.datetime.now()
time_one_hour_ago = time_now - datetime.timedelta( hours = 1 )
hour = time_one_hour_ago.strftime( "%H" )
day = time_one_hour_ago.strftime( "%m%d" )
year = time_one_hour_ago.strftime( "%Y" )
encode_hour( year, day, hour )

