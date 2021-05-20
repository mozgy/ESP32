#!/usr/bin/python

#
# v1.05
#

import os
import tempfile
import cv2
import numpy
import datetime
import subprocess

camIP = '192.168.1.66'    # Cam1


def encode_hour( f_year, f_day, f_hour ):

  ## Fix_01?
  pic_dir = '{}{}'.format( f_year, f_day )
  if not os.path.exists( pic_dir ):
    os.makedirs( pic_dir )
  os.chdir( './{}'.format( pic_dir ) )

  cmd_call = 'wget --quiet --recursive --no-directories --no-host-directories --timestamping --level=1 --accept="PIC*jpg" "http://{}/ai-cam/{}/{}"'.format( camIP, f_day, f_hour )
  subprocess.call( cmd_call, shell=True )
  # print( cmd_call )

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

    cmd_call = 'cat {} | ffmpeg -f image2pipe -i - ../cam1-{}{}.mkv'.format( file_list, f_day, f_hour )
    subprocess.call( cmd_call, shell=True )
    # print( cmd_call )

  ## Fix_01?
  os.chdir( '..' )



def encode_day( f_year, f_day ):

  file_list = ''
  aicam_dir = os.getcwd()
  sorted_ls = sorted( os.listdir( aicam_dir ) )

  for aicam_fn in sorted_ls:

    if aicam_fn.startswith( 'cam1-{}'.format( f_day ) ):

      file_list = file_list + "file '{}/{}'\n".format( aicam_dir, aicam_fn )
      continue

  if file_list != '':

    fd, f_path = tempfile.mkstemp()
    with os.fdopen( fd, 'w' ) as tmp:
      tmp.write( file_list )

    cmd_call = 'ffmpeg -f concat -safe 0 -i {} -c copy {}/cam1-{}{}.mkv'.format( f_path, aicam_dir, f_year, f_day )
    subprocess.call( cmd_call, shell=True )
    # print( cmd_call )

    os.remove( f_path )



def fetch_whole_day( f_year, f_day ):

  for i_hour in range( 24 ):
    f_hour = '{:02}'.format( i_hour )
    encode_hour( f_year, f_day, f_hour )

  encode_day( f_year, f_day )


### Main Loop

time_now = datetime.datetime.now()
time_one_hour_ago = time_now - datetime.timedelta( hours = 1 )
hour = time_one_hour_ago.strftime( "%H" )
day = time_one_hour_ago.strftime( "%m%d" )
year = time_one_hour_ago.strftime( "%Y" )

encode_hour( year, day, hour )

