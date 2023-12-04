#!/usr/bin/python

#
# v1.12
#

import os, tempfile, subprocess
import sys, getopt
import datetime

camIP1 = '192.168.1.66'
camIP2 = '192.168.1.67'
camIP3 = '192.168.1.68'
camIP4 = '192.168.1.69'
camIP = camIP1

check_mode = False


def encode_hour( f_year, f_day, f_hour ):

  import cv2, numpy

  pic_dir = '{}{}'.format( f_year, f_day )
  if not os.path.exists( pic_dir ):
    os.makedirs( pic_dir )
  os.chdir( './{}'.format( pic_dir ) )

  cmd_call = 'wget --quiet --recursive --no-directories --no-host-directories --timestamping --level=1 --accept="PIC*jpg" "http://{}/ai-cam/{}/{}"'.format( camIP, f_day, f_hour )
  if check_mode:
    print( cmd_call )
  else:
    subprocess.call( cmd_call, shell=True )

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
    if check_mode:
      print( cmd_call )
    else:
      subprocess.call( cmd_call, shell=True )

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
    if check_mode:
      print( cmd_call )
    else:
      subprocess.call( cmd_call, shell=True )

    os.remove( f_path )



def fetch_whole_day( f_year, f_day ):

  for i_hour in range( 24 ):
    f_hour = '{:02}'.format( i_hour )
    encode_hour( f_year, f_day, f_hour )

  encode_day( f_year, f_day )



def delete_hour( f_year, f_day, f_hour ):

  cmd_call = 'curl "http://{}/ai-cam/{}/{}"'.format( camIP, f_day, f_hour )
  if check_mode:
    print( cmd_call )
  else:
    subprocess.call( cmd_call, shell=True )

  cmd_list = cmd_call.partition( "href=" )
  print( cmd_list )



### Main Loop

full_cmd_list = sys.argv
argument_list = full_cmd_list[1:]
short_options = 'htm:d:o:c:'
long_options = [ 'help', 'test', 'mode=', 'date=', 'output=', 'camera=' ]

mode = ''
fetch_date = '00-00-0000'

try:
  args, vals = getopt.getopt( argument_list, short_options, long_options )
  for curr_arg, curr_val in args:
    # print( 'for loop: {}.{}'.format( curr_arg, curr_val ) )
    if curr_arg in ( '-h', '--help' ):
      print( 'ToDo: print help' )

    elif curr_arg in ( '-t', '--test' ):
      check_mode = True
      print( 'Check-no-exe mode active!' )

    elif curr_arg in ( '-m', '--mode' ):
      mode = curr_val

    elif curr_arg in ( '-d', '--date' ):
      fetch_date = check_date( curr_val )

    elif curr_arg in ( "-o", "--output" ):
      storage_dir = curr_val

    elif curr_arg in ( "-c", "--camera" ):
      cam_num = curr_val

except getopt.error as err:
  print( str( err ) )
  sys.exit( 2 )

if check_mode:
  storage_dir = '/var/www/html/Pictures'
else:
  storage_dir = '/tmp'
os.chdir( storage_dir )

time_now = datetime.datetime.now()

if mode == 'hourly':
  time_ago = time_now - datetime.timedelta( hours = 1 )
  hour = time_ago.strftime( "%H" )
  day = time_ago.strftime( "%m%d" )
  year = time_ago.strftime( "%Y" )
  print( 'Processing Time - {}-{}-{}'.format( hour, day, year ) )
  encode_hour( year, day, hour )
elif mode == 'daily':
  time_ago = time_now - datetime.timedelta( hours = 24 )
  hour = time_ago.strftime( "%H" )
  day = time_ago.strftime( "%m%d" )
  year = time_ago.strftime( "%Y" )
  print( 'Processing Time - {}-{}-{}'.format( hour, day, year ) )
  encode_day( year, day )
elif mode == 'fetch':
  print( 'TODO - fetch mode if' )
elif mode == 'delete':
  delete_hour( '2021', '0430', '20' )
else:
  print( 'Correct Mode Missing' )
