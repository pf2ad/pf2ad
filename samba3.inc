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

# ------------------------------------------------------------------------------
# Load the javascript jquery lib
# ------------------------------------------------------------------------------

function change_role() {

    $javascript = <<<EOD
<script language="JavaScript">
<!--
function do_role() {
    switch ($("#server_role").val()) {
        case 'dc':
            $("tr>td>#interface").parent().parent().show();

EOD;
    if (file_exists(DC_BASE)) {
        $javascript .= <<<EOD
            // DC
            $("td[class='listtopic']:contains('Domain Controller Options'):first").html("<table width='100%'><tr><td>Domain Controller Options</td><td align='right'><button id='remove_config' name='remove_config' type='button'>Remove config</button></td></tr></table>").show();
            $("tr>td>#dc_domain").parent().parent().show();
            $("#dc_domain").attr("readonly", "readonly");
            $("tr>td>#dc_workgroup").parent().parent().show();
            $("#dc_workgroup").attr("readonly", "readonly");
            $("tr>td>#dc_password").parent().parent().show();
            $("#dc_password").attr("readonly", "readonly");

            $("option[value='join_dc']").attr("disabled", "disabled");
            $("option[value='rodc']").attr("disabled", "disabled");
            $("option[value='member']").attr("disabled", "disabled");
EOD;
    } else {
        $javascript .= <<< EOD
            // DC
            $("tr>td[class='listtopic']:contains('Domain Controller Options')").show();
            $("tr>td>#dc_domain").parent().parent().show();
            $("#dc_domain").focus();
            $("tr>td>#dc_workgroup").parent().parent().show();
            $("tr>td>#dc_password").parent().parent().show();
EOD;
    }

    $javascript .= <<< EOD

            // enable tabs
            $("li:contains('DNS Server')").show();
            $("li:contains('User Mgmt')").show();
            $("li:contains('Shares')").show();

            // DC Additional
            $("tr>td[class='listtopic']:contains('Additional Domain Controller Options')").hide();
            $("tr>td>#adc_domain").parent().parent().hide();
            $("tr>td>#adc_server").parent().parent().hide();
            $("tr>td>#adc_site").parent().parent().hide();
            $("tr>td>#adc_username").parent().parent().hide();
            $("tr>td>#adc_password").parent().parent().hide();

            // RODC
            $("tr>td[class='listtopic']:contains('RODC Options')").hide();
            $("tr>td>#rodc_domain").parent().parent().hide();
            $("tr>td>#rodc_server").parent().parent().hide();
            $("tr>td>#rodc_site").parent().parent().hide();
            $("tr>td>#rodc_username").parent().parent().hide();
            $("tr>td>#rodc_password").parent().parent().hide();

            // Domain Member
            $("tr>td[class='listtopic']:contains('Domain Member')").hide();
            $("tr>td>#member_domain").parent().parent().hide();
            $("tr>td>#member_workgroup").parent().parent().hide();
            $("tr>td>#member_username").parent().parent().hide();
            $("tr>td>#member_password").parent().parent().hide();

            break;

        case 'join_dc':
            $("tr>td>#interface").parent().parent().show();

            // enable tabs
            $("li:contains('DNS Server')").show();
            $("li:contains('User Mgmt')").show();
            $("li:contains('Shares')").show();

            // DC
            $("tr>td[class='listtopic']:contains('Domain Controller Options')").hide();
            $("tr>td>#dc_domain").parent().parent().hide();
            $("tr>td>#dc_workgroup").parent().parent().hide();
            $("tr>td>#dc_password").parent().parent().hide();

            // DC Additional
            $("tr>td[class='listtopic']:contains('Additional Domain Controller Options')").show();
            $("tr>td>#adc_domain").parent().parent().show();
            $("tr>td>#adc_server").parent().parent().show();
            $("tr>td>#adc_site").parent().parent().show();
            $("tr>td>#adc_username").parent().parent().show();
            $("tr>td>#adc_password").parent().parent().show();

            // RODC
            $("tr>td[class='listtopic']:contains('RODC Options')").hide();
            $("tr>td>#rodc_domain").parent().parent().hide();
            $("tr>td>#rodc_server").parent().parent().hide();
            $("tr>td>#rodc_site").parent().parent().hide();
            $("tr>td>#rodc_username").parent().parent().hide();
            $("tr>td>#rodc_password").parent().parent().hide();

            // Domain Member
            $("tr>td[class='listtopic']:contains('Domain Member')").hide();
            $("tr>td>#member_domain").parent().parent().hide();
            $("tr>td>#member_workgroup").parent().parent().hide();
            $("tr>td>#member_username").parent().parent().hide();
            $("tr>td>#member_password").parent().parent().hide();

            break;

        case 'rodc':
            $("tr>td>#interface").parent().parent().show();

            // enable tabs
            $("li:contains('DNS Server')").show();
            $("li:contains('User Mgmt')").show();
            $("li:contains('Shares')").show();

            // DC
            $("tr>td[class='listtopic']:contains('Domain Controller Options')").hide();
            $("tr>td>#dc_domain").parent().parent().hide();
            $("tr>td>#dc_workgroup").parent().parent().hide();
            $("tr>td>#dc_password").parent().parent().hide();

            // DC Additional
            $("tr>td[class='listtopic']:contains('Additional Domain Controller Options')").hide();
            $("tr>td>#adc_domain").parent().parent().hide();
            $("tr>td>#adc_server").parent().parent().hide();
            $("tr>td>#adc_site").parent().parent().hide();
            $("tr>td>#adc_username").parent().parent().hide();
            $("tr>td>#adc_password").parent().parent().hide();

            // RODC
            $("tr>td[class='listtopic']:contains('RODC Options')").show();
            $("tr>td>#rodc_domain").parent().parent().show();
            $("tr>td>#rodc_server").parent().parent().show();
            $("tr>td>#rodc_site").parent().parent().show();
            $("tr>td>#rodc_username").parent().parent().show();
            $("tr>td>#rodc_password").parent().parent().show();

            // Domain Member
            $("tr>td[class='listtopic']:contains('Domain Member')").hide();
            $("tr>td>#member_domain").parent().parent().hide();
            $("tr>td>#member_workgroup").parent().parent().hide();
            $("tr>td>#member_username").parent().parent().hide();
            $("tr>td>#member_password").parent().parent().hide();

            break;

        case 'member':
            $("tr>td>#interface").parent().parent().show();

            // disable tabs
            $("li:contains('DNS Server')").hide();
            $("li:contains('User Mgmt')").hide();
            $("li:contains('Shares')").show();

            // DC
            $("tr>td[class='listtopic']:contains('Domain Controller Options')").hide();
            $("tr>td>#dc_domain").parent().parent().hide();
            $("tr>td>#dc_workgroup").parent().parent().hide();
            $("tr>td>#dc_password").parent().parent().hide();

            // DC Additional
            $("tr>td[class='listtopic']:contains('Additional Domain Controller Options')").hide();
            $("tr>td>#adc_domain").parent().parent().hide();
            $("tr>td>#adc_server").parent().parent().hide();
            $("tr>td>#adc_site").parent().parent().hide();
            $("tr>td>#adc_username").parent().parent().hide();
            $("tr>td>#adc_password").parent().parent().hide();

            // RODC
            $("tr>td[class='listtopic']:contains('RODC Options')").hide();
            $("tr>td>#rodc_domain").parent().parent().hide();
            $("tr>td>#rodc_server").parent().parent().hide();
            $("tr>td>#rodc_site").parent().parent().hide();
            $("tr>td>#rodc_username").parent().parent().hide();
            $("tr>td>#rodc_password").parent().parent().hide();

            // Domain Member
            $("tr>td[class='listtopic']:contains('Domain Member')").show();
            $("tr>td>#member_domain").parent().parent().show();
            $("tr>td>#member_workgroup").parent().parent().show();
            $("tr>td>#member_username").parent().parent().show();
            $("tr>td>#member_password").parent().parent().show();

            break;

        default:
            $("tr>td>#interface").parent().parent().hide();
EOD;

    if (file_exists(DC_BASE)) {
        $javascript .= <<<EOD
            $("option[value='join_dc']").attr("disabled", "disabled");
            $("option[value='rodc']").attr("disabled", "disabled");
            $("option[value='member']").attr("disabled", "disabled");
EOD;
    }
    $javascript .= <<<EOD

            // disable tabs
            $("li:contains('DNS Server')").hide();
            $("li:contains('User Mgmt')").hide();
            $("li:contains('Shares')").hide();

            // DC
            $("tr>td[class='listtopic']:contains('Domain Controller Options')").hide();
            $("tr>td>#dc_domain").parent().parent().hide();
            $("tr>td>#dc_workgroup").parent().parent().hide();
            $("tr>td>#dc_password").parent().parent().hide();

            // DC Additional
            $("tr>td[class='listtopic']:contains('Additional Domain Controller Options')").hide();
            $("tr>td>#adc_domain").parent().parent().hide();
            $("tr>td>#adc_server").parent().parent().hide();
            $("tr>td>#adc_site").parent().parent().hide();
            $("tr>td>#adc_username").parent().parent().hide();
            $("tr>td>#adc_password").parent().parent().hide();

            // RODC
            $("tr>td[class='listtopic']:contains('RODC Options')").hide();
            $("tr>td>#rodc_domain").parent().parent().hide();
            $("tr>td>#rodc_server").parent().parent().hide();
            $("tr>td>#rodc_site").parent().parent().hide();
            $("tr>td>#rodc_username").parent().parent().hide();
            $("tr>td>#rodc_password").parent().parent().hide();

            // Domain Member
            $("tr>td[class='listtopic']:contains('Domain Member')").hide();
            $("tr>td>#member_domain").parent().parent().hide();
            $("tr>td>#member_workgroup").parent().parent().hide();
            $("tr>td>#member_username").parent().parent().hide();
            $("tr>td>#member_password").parent().parent().hide();

            break;
    }
}
$(function() {
    $("#server_role").change(function(){
        do_role();
    });
    do_role();

    $("#remove_config").click(function() {
        $("form[name='iform']").append('<input type="hidden" name="del_config" value="true">');
        $("#server_role").val("disabled");
        $("form[name='iform']").submit();
    });

});

-->
</script>
EOD;

    print $javascript;

}


function get_real_interface_address($iface) {
    global $config;

    $iface = convert_friendly_interface_to_real_interface_name($iface);
    $line = trim(shell_exec("ifconfig $iface | grep inet | grep -v inet6"));
    list($dummy, $ip, $dummy2, $netmask) = explode(" ", $line);

    return array($ip, long2ip(hexdec($netmask)));
}

function validate_form_samba3($post, &$input_errors) {

    if ($post['del_config'] == 'true') {
        mwexec("/usr/local/etc/rc.d/samba stop");
        sleep(2);
        mwexec('/bin/rm -rf /var/db/samba3/private/*');
        mwexec('/bin/rm -rf /usr/local/etc/smb.conf');
    } else {

        // Domain Controller
        if ( $post['server_role'] == 'dc' ) {
            if ( $post['dc_domain'] == "" )
                $input_errors[] = "The field 'Domain' is required.";
            if ( $post['dc_workgroup'] == "" )
                $input_errors[] = "The field 'Workgroup' is required.";
            if ( $post['dc_password'] == "" )
                $input_errors[] = "The field 'Administrator Password' is required.";
        }
        // Additional Domain Controller
        if ( $post['server_role'] == 'join_dc' ) {
            if ( $post['adc_domain'] == "" )
                $input_errors[] = "The field 'Domain' is required.";
            if ( $post['adc_username'] == "" )
                $input_errors[] = "The field 'Username Administrator' is required.";
            if ( $post['adc_password'] == "" )
                $input_errors[] = "The field 'Administrator Password' is required.";
        }
        // RODC
        if ( $post['server_role'] == 'rodc' ) {
            if ( $post['rodc_domain'] == "" )
                $input_errors[] = "The field 'Domain' is required.";
            if ( $post['rodc_username'] == "" )
                $input_errors[] = "The field 'Username Administrator' is required.";
            if ( $post['rodc_password'] == "" )
                $input_errors[] = "The field 'Administrator Password' is required.";
        }
        // Member
        if ( $post['server_role'] == 'member' ) {
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
}

function sync_samba3() {
    global $g, $config;

    $settings = $config['installedpackages']['samba3']['config'][0];

    if ($settings['server_role'] == 'disabled') {
        // stop any process if exist samba3.sh
        if (file_exists("/usr/local/etc/rc.d/samba.sh")) {
            mwexec("/usr/local/etc/rc.d/samba3.sh stop");
            mwexec("rm -rf /usr/local/etc/rc.d/samba3.sh");
        }
    }

    if ($settings['server_role'] == 'dc') {
        $domain = $settings['dc_domain'];
        $workgroup = strtoupper($settings['dc_workgroup']);
        $adminpass = $settings['dc_password'];
        $hostip = get_real_interface_address($settings['interface']);

        // stop any process if exist samba3.sh
        if (file_exists("/usr/local/etc/rc.d/samba3.sh")) {
            mwexec("/usr/local/etc/rc.d/samba3.sh stop");
        }

        if (! file_exists(DC_BASE)) {
            mwexec("rm -rf /usr/local/etc/smb4.conf");
            mwexec("/usr/local/bin/samba-tool domain provision --domain=" . $workgroup . " --realm=". $domain . " --server-role=dc --adminpass=" . $adminpass . " --host-ip=" . $hostip[0] . " --quiet");
            mwexec("cd /etc && rm -rf krb5.conf && ln -s /var/db/samba3/private/krb5.conf");
        }

        samba_write_rcfile();
        if (!is_service_running('samba3')) {
            log_error("Starting samba3");
            mwexec("/usr/local/etc/rc.d/samba3.sh start");
        } else {
            log_error("Reloading samba3 for configuration sync");
            mwexec("/usr/local/etc/rc.d/samba3.sh stop");
            sleep(2);
            mwexec("/usr/local/etc/rc.d/samba3.sh start");
        }

    }

    if ($settings['server_role'] == 'member') {
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
   winbind enum users  = yes
   winbind enum groups = yes

EOD;

        file_put_contents(SAMBA3CONF, $smb_conf);
        samba_write_rcfile();
        mwexec("/usr/local/bin/net ads join -U '" . $username . "%" . $adminpass . "'");

        if (!is_service_running('smbd')) {
            log_error("Starting samba3");
            mwexec("/usr/local/etc/rc.d/samba start");
        } else {
            log_error("Reloading samba3 for configuration sync");
            mwexec("/usr/local/etc/rc.d/samba stop");
            sleep(2);
            mwexec("/usr/local/etc/rc.d/samba start");
        }
    }

    if ($settings['server_role'] == 'disabled') {
        print "teste de log";
    }

}

function samba_write_rcfile() {
    global $g, $config;

    $settings = $config['installedpackages']['samba3']['config'][0];

    if ($settings['server_role'] == 'member') {

        $rc = array();
        $rc['file'] = 'samba3.sh';
        $rc['start'] = <<<EOD
if [ -z "`ps axuw | egrep "nmbd|smbd|winbindd" | grep -v 'grep' |awk '{print $2}'`" ];then
	/usr/local/sbin/smbd -D -s /usr/local/etc/smb4.conf
    /usr/local/sbin/nmbd -D -s /usr/local/etc/smb4.conf
    /usr/local/sbin/winbindd -D -s /usr/local/etc/smb4.conf
fi

EOD;
        $rc['stop'] = <<<EOD
killall nmbd 2>/dev/null
killall smbd 2>/dev/null
killall winbindd 2>/dev/null

EOD;

    } elseif ( $settings['server_role'] == 'dc' ) {
        $rc = array();
        $rc['file'] = 'samba3.sh';
        $rc['start'] = <<<EOD
if [ -z "`ps axuw | egrep "/usr/local/sbin/samba3 -D" | grep -v 'grep' |awk '{print $2}'`" ];then
    if [ ! -d "/var/run/samba3" ]; then
        mkdir -p /var/run/samba3
    fi
    /usr/local/sbin/samba3 -D -s /usr/local/etc/smb4.conf
fi

EOD;
        $rc['stop'] = <<<EOD
killall samba3

EOD;
    }

    // write rc file
	conf_mount_rw();
	write_rcfile($rc);
	conf_mount_ro();
}

function do_remove_config() {
    mwexec("/usr/local/etc/rc.d/samba stop");
    sleep(2);
    mwexec("rm -rf /var/db/samba3/private/*");
    mwexec("rm -rf /usr/local/etc/smb.conf");
}

?>
