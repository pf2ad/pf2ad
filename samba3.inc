<?php
# ------------------------------------------------------------------------------
/*  samba3.inc
    Copyright (C) 2013-2016 Luiz Gustavo <me@luizgustavo.pro.br>
    All rights reserved.

    part of pfSense (www.pfSense.com)

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
    AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/
# ------------------------------------------------------------------------------

require_once('globals.inc');
require_once('config.inc');
require_once('util.inc');
require_once('pfsense-utils.inc');
require_once('pkg-utils.inc');
require_once('filter.inc');
require_once('service-utils.inc');


define('SAMBA3CONF', '/usr/local/etc/smb.conf');
define('DC_BASE', "/var/db/samba3/private/sam.ldb");

function get_real_interface_address($iface) {
    global $config;

    $iface = convert_friendly_interface_to_real_interface_name($iface);
    $line = trim(shell_exec("ifconfig $iface | grep inet | grep -v inet6"));
    list($dummy, $ip, $dummy2, $netmask) = explode(" ", $line);

    return array($ip, long2ip(hexdec($netmask)));
}

function validate_form_samba3($post, &$input_errors) {
    // Member
    if (isset($post['samba3enable'])) {
        if ( $post['member_domain'] == "" )
            $input_errors[] = "The field 'Domain' is required.";
        if ( $post['member_workgroup'] == "" )
            $input_errors[] = "The field 'Workgroup' is required.";
        if ( $post['member_username'] == "" )
            $input_errors[] = "The field 'Username Administrator' is required.";
        if ( $post['member_password'] == "" )
            $input_errors[] = "The field 'Administrator Password' is required.";
    }
}

function sync_samba3() {
    global $g, $config;

    if (is_array($config['installedpackages']['samba3'])) {
      $settings = $config['installedpackages']['samba3']['config'][0];
		  if ($settings['samba3enable'] == "on") {
        $domain = strtoupper($settings['member_domain']);
        $workgroup = strtoupper($settings['member_workgroup']);
        $username = $settings['member_username'];
        $adminpass = $settings['member_password'];
        $hostip = get_real_interface_address($settings['interface']);
        $krb_conf = <<<EOD
[logging]
     default = FILE:/var/log/krb5libs.log
     kdc = FILE:/var/log/krb5kdc.log
     admin_server = FILE:/var/log/kadmind.log

[libdefaults]
     default_realm = $domain
     dns_lookup_realm = true
     dns_lookup_kdc = true
     ticket_lifetime = 24h
     forwardable = yes

[appdefaults]
     pam = {
          debug = false
          ticket_lifetime = 36000
          renew_lifetime = 36000
          forwardable = true
          krb4_convert = false
     }

EOD;
        file_put_contents('/etc/krb5.conf', $krb_conf);
        $smb_conf = <<<EOD
[global]

   workgroup = $workgroup
   security = ADS
   realm = $domain
   encrypt passwords = yes

   interfaces = $hostip[0]

   idmap config *:backend = tdb
   idmap config *:range = 70001-80000
   idmap config $workgroup:backend = ad
   idmap config $workgroup:schema_mode = rfc2307
   idmap config $workgroup:range = 500-40000

   winbind nss info = rfc2307
   winbind trusted domains only = no
   winbind use default domain = yes
   winbind enum users  = no
   winbind enum groups = no

EOD;
        file_put_contents(SAMBA3CONF, $smb_conf);
        samba_write_rcfile();
        mwexec("/usr/local/bin/net ads join -U '" . $username . "%" . $adminpass . "'");
        if (!is_service_running('smbd')) {
          log_error("Starting samba3");
          mwexec("/usr/local/etc/rc.d/samba3.sh start");
        } else {
          log_error("Reloading samba3 for configuration sync");
          mwexec("/usr/local/etc/rc.d/samba3.sh stop");
          sleep(2);
          mwexec("/usr/local/etc/rc.d/samba3.sh start");
        }
      } else {
        // stop any process if exist samba3.sh
        if (file_exists("/usr/local/etc/rc.d/samba3.sh")) {
          mwexec("/usr/local/etc/rc.d/samba3.sh stop");
          mwexec("rm -rf /usr/local/etc/rc.d/samba3.sh");
        }
      }
    }
}

function samba_write_rcfile() {
    global $g, $config;

    if (is_array($config['installedpackages']['samba3'])) {
      $settings = $config['installedpackages']['samba3']['config'][0];
		  if ($settings['samba3enable'] == "on") {
        $samba_conf = SAMBA3CONF;
        $rc = array();
        $rc['file'] = 'samba3.sh';
        $rc['start'] = <<<EOD
if [ -z "`ps axuw | egrep "nmbd|smbd|winbindd" | grep -v 'grep' |awk '{print $2}'`" ];then
  mkdir /var/run/samba
  /usr/local/sbin/smbd -D -s ${samba_conf}
  /usr/local/sbin/nmbd -D -s ${samba_conf}
  /usr/local/sbin/winbindd -D -s ${samba_conf}
fi

EOD;
        $rc['stop'] = <<<EOD
killall nmbd 2>/dev/null
killall smbd 2>/dev/null
killall winbindd 2>/dev/null

EOD;
        // write rc file
	      conf_mount_rw();
	      write_rcfile($rc);
	      conf_mount_ro();
      }
    }
}

?>
