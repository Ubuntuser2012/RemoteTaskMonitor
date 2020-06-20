import rtm

def main():

	count = 0
	while(count < 100 and False == do_connect()):
		count=count+1
		print("Try to connect again")
		do_connect()
	rtm.disconnect()
	return 0;

def do_connect():
	connected = False
	if(0==rtm.connect("127.0.0.1", 8291)):
		print("Check IP and port, and check that you are not already connected.")
	else:
		loop = True
		count = 0
		attached = False
		
		# Get x snapshots of the target device
		while (loop and count < 100):
			r = rtm.wait(-1)
			#print(r)
			if (r['reason'] == 'quit' or r['reason'] == 'disconnected'):
				loop = False
				connected = False
				print("Could not connect to target. Check your IP/Port, cemon is running on target device and a TCP/IP connection is available.")
			elif (r['reason'] == 'connected'):
				rtm.startcapture()
				rtm.enablepsmemstats(1)
				count = 0
				connected = True
				print("Connected")
			elif (r['reason'] == 'gotsnapshot'):
				process_snapshot(attached)
			elif (r['reason'] == 'crashwatchack'):
				attached = True
				print('attached')
			elif (r['reason'] == 'crashwatchnack'):
				attached = False
				print('not attach')
			elif (r['reason'] == 'exception'):
				attached = False
				print('exception')
				print(r['pspath'])
				print(r['callstack'])
				loop = False
			count=count+1
	print("Returning now - state of connection: " + str(connected))
	return connected

	
def process_snapshot(attached):
	msg = 'Total CPU '+ str(rtm.gettotalcpu()) + '%, Total memory ' + str(rtm.getavailmem()) + ' Bytes'
	print(msg)
	
	#Get list of all processes
	pss = rtm.getpss()
	#print(pss)
	if(pss):
		for ps in pss: # Iterate though each process in the list
			#print(ps)
			if(ps['name'] == 'test.exe'):
				#print('test.exe CPU usage: ' + str(ps['cpu']) + '%')
				if(False == attached):
					print('try to attach to ' + ps['name'] + ':' + str(ps['pid']))
					#Attach to process
					pid = ps['pid'];
					rtm.crashwatchps(pid, '')

	return 0;
