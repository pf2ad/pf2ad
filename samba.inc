<?php
# ------------------------------------------------------------------------------
/*  samba.inc
    Copyright (C) 2013-2016 Luiz Gustavo <me@luizgustavo.pro.br>
    All rights reserved.
*/
# ------------------------------------------------------------------------------

require_once('globals.inc');
require_once('config.inc');
require_once('util.inc');
require_once('pfsense-utils.inc');
require_once('pkg-utils.inc');
require_once('filter.inc');
require_once('service-utils.inc');


define('SAMBACONF', '/usr/local/etc/smb4.conf');
define('DC_BASE', "/var/db/samba4/private/sam.ldb");

function get_real_interface_address($iface) {
    global $config;

    $iface = convert_friendly_interface_to_real_interface_name($iface);
    $line = trim(shell_exec("ifconfig $iface | grep inet | grep -v inet6"));
    list($dummy, $ip, $dummy2, $netmask) = explode(" ", $line);

    return array($ip, long2ip(hexdec($netmask)));
}

function validate_form_samba($post, &$input_errors) {
    // Member
    if (isset($post['samba_enable'])) {
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

function sync_samba() {
    global $g, $config;

    if (is_array($config['installedpackages']['samba'])) {
      $settings = $config['installedpackages']['samba']['config'][0];
		  if ($settings['samba_enable'] == "on") {
        $domain = strtoupper($settings['member_domain']);
        $domain_low = strtolower($settings['member_domain']);
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

EOD;
if ($settings['ad_version'] == 'win2003') {
  $krb_conf .= <<<EOD
; for Windows 2003
    default_tgs_enctypes = rc4-hmac des-cbc-crc des-cbc-md5
    default_tkt_enctypes = rc4-hmac des-cbc-crc des-cbc-md5
    permitted_enctypes = rc4-hmac des-cbc-crc des-cbc-md5
EOD;
}
if ($settings['ad_version'] == 'win2008') {
  $krb_conf .= <<<EOD
; for Windows 2008 with AES
    default_tgs_enctypes = aes256-cts-hmac-sha1-96 rc4-hmac des-cbc-crc des-cbc-md5
    default_tkt_enctypes = aes256-cts-hmac-sha1-96 rc4-hmac des-cbc-crc des-cbc-md5
    permitted_enctypes = aes256-cts-hmac-sha1-96 rc4-hmac des-cbc-crc des-cbc-md5
EOD;
}

$krb_conf .= <<<EOD

[appdefaults]
    pam = {
        debug = false
        ticket_lifetime = 36000
        renew_lifetime = 36000
        forwardable = true
        krb4_convert = false
    }

[domain_realm]
    .$domain_low = $domain
    $domain_low = $domain

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
        file_put_contents(SAMBACONF, $smb_conf);
        samba_write_rcfile();
        $ret = exec_command("/usr/local/bin/net ads join -U '" . $username . "%" . $adminpass . "'");
        if (strpos($ret[0], 'Joined') !== false) {
          if (!is_service_running('samba')) {
            sleep(2);
            log_error("Starting Samba");
            mwexec("/usr/local/etc/rc.d/samba.sh start");
          } else {
            log_error("Reloading Samba for configuration sync");
            mwexec("/usr/local/etc/rc.d/samba.sh stop");
            sleep(2);
            mwexec("/usr/local/etc/rc.d/samba.sh start");
          }
        } else {
          // Deu erro.. tratar...
        }
      } else {
        // stop any process if exist samba.sh
        if (file_exists("/usr/local/etc/rc.d/samba.sh")) {
          mwexec("/usr/local/etc/rc.d/samba.sh stop");
          mwexec("rm -rf /usr/local/etc/rc.d/samba.sh");
        }
      }
      // Create key tab file (kerberos)
      if (!file_exists('/etc/krb5.keytab')) {
        mwexec("/usr/local/bin/net ads keytab create -U '" . $username . "%" . $adminpass . "'");
        mwexec("/usr/sbin/chown :proxy /etc/krb5.keytab");
        mwexec("/bin/chmod 0750 /etc/krb5.keytab");
      }
    }
}

function samba_write_rcfile() {
    global $g, $config;

    if (is_array($config['installedpackages']['samba'])) {
      $settings = $config['installedpackages']['samba']['config'][0];
		  if ($settings['samba_enable'] == "on") {
        $samba_conf = SAMBACONF;
        $rc = array();
        $rc['file'] = 'samba.sh';
        $rc['start'] = <<<EOD
if [ -z "`ps axuw | egrep "nmbd|smbd|winbindd" | grep -v 'grep' |awk '{print $2}'`" ];then
  mkdir /var/run/samba4
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
