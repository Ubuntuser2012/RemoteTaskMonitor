import rtm

def main():
	if(0==rtm.connect("127.0.0.1", 8291)):
		print("Check IP and port, and check that you are not already connected.")
	else:
		loop = True
		count = 0
		connected = False
		
		# Get 20 snapshots of the target device
		while (loop and count < 20):
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
				process_snapshot()
			count=count+1
		
		rtm.resetdevice()
		# Disconnect and get the report for the session
		report = ''
		if(connected):
			rtm.disconnect()
			print("Disconnecting")
			r = rtm.wait(-1) # Wait to be disconnected
			if(r['reason'] == 'disconnected'):
				report = r['report']
				print(report)
		# Send an email with path to the report file
		rtm.email("foo@bar.com", "RTM Report", "Report is here: " + report)	
		
	return 0;

def process_snapshot():
	msg = 'Total CPU '+ str(rtm.gettotalcpu()) + '%, Total memory ' + str(rtm.getavailmem()) + ' Bytes'
	print(msg)
	
	#Get list of all processes
	pss = rtm.getpss()
	#print(pss)
	if(pss):
		for ps in pss: # Iterate though each process in the list
			#print(ps)
			if(ps['name'] == 'pcmon.exe'):
				print('pcmon.exe CPU usage: ' + str(ps['cpu']) + '%')
				#Get list of all threads in this process
				ths = rtm.getthreads(ps['pid'])
				if(ths):
					for th in ths: # Iterate though each thread in the list
						#print(th)
						if(th):
							print('Running at priority ' + str(th['priority']))

	return 0;

