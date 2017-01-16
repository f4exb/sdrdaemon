#
# Regular cron jobs for the sdrdaemon package
#
0 4	* * *	root	[ -x /usr/bin/sdrdaemon_maintenance ] && /usr/bin/sdrdaemon_maintenance
