## Setup SDRdaemon as a service ##

<h2>sdrdaemonrx</h2>

<h3>Pre-requisites</h3>

You have first to install sdrdaemon package in `/opt/sdrdaemon`. This can be done by compiling the source and specifying this directory as the installation target or installing the package with the `.deb` file. If you choose any different installation directory you have to update the exacutable full path in `DAEMON` variable in the `/etc/init.d/sdrdaemonrx` 

You also need `jq` a "lightweight and flexible command-line JSON processor":

`sudo apt-get install jq`

<h3>Install the files</h3>

The installation simply consists of two files with variants:

  - `sdrdaemonrx_debian`: this is the service script to be copied (with sudo) to `/etc/init.d` in Debian/Ubuntu installations
  - `sdrdaemonrx_opensuse`: this is the service script to be copied (with sudo) to `/etc/init.d` in openSUSE installations
  - `sdrdaemonrx.conf`: this is a configuration file be copied (with sudo) as `/var/lib/sdrdaemon/sdrdaemonrx.conf` in all installations
  
To install the files you do:

<pre>
sudo cp sdrdaemonrx\_<variant> /etc/init.d/sdrdaemonrx
sudo chmod +x /etc/init.d/sdrdaemonrx
sudo mkdir -p /var/lib/sdrdaemon
sudo cp sdrdaemonrx.conf /var/lib/sdrdaemon
</pre>

<h3>Setup the service</h3>

<h4>Debian/Ubuntu</h4>

<pre>
sudo service sdrdaemonrx defaults
sudo service sdrdaemonrx enable
</pre>

<h4>openSUSE</h4>

<pre>
sudo systemctl enable sdrdaemonrx
</pre>

<h3>Configuration file</h3>

The configuration file `sdrdaemonrx.conf` is a simple JSON fragment:

<pre>
{
    "type": "rtlsdr",
    "ip": "192.168.1.3",
    "dport": 9094,
    "cport": 9095,
    "cmd": "txdelay=750,fecblk=8,freq=430060000,srate=256000,gain=49.6,fcpos=2,decim=2,ppmp=69"
}
</pre>

Where:

  - type: is the type of hardware as in the `-t` argument
  - ip: is the IP address of destination as in the `-I` argument
  - dport: is the destination data port as in the `-D` argument
  - cport: is the listening control port as in the `-C` argument
  - cmd: is the device command line as `-c` argument
  
<h3>Manage the service</h3>
  
<h4>Debian/Ubuntu</h4>

You can manage the service with the `service` command like any other service:

`sudo service <cmd> sdrdaemonrx`

Where `<cmd>` is any of the following commands:

  - `start`: start the service
  - `stop`: stop the service
  - `restart`: retart the service (equivalent of `stop` followed by `start`)
  - `force-reload`: same as `restart`
  - `status`: display service status. Useful to get the tail of the log in case of a problem.
  
<h4>openSUSE</h4>

You can manage the service with 'systemctl' command:

'sudo systemctl <cmd> sdrdaemonrx'

The <cmd> commands are the same as for Debian/Ubuntu 

<h3>Remove the service</h3>

<h4>Debian/Ubintu</h4>

<pre>
sudo update-rc.d -f service-name remove
</pre>

<h4>openSUSE</h4>

<pre>
sudo systemctl disable sdrdaemonrx
sudo systemctl reset-failed
</pre>

You can then remove (rename) the files
