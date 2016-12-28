#!/bin/sh

VERSION='20161228008' # Happy new year 2017 !

if [ -f "/etc/samba3.patch.version" ]; then
	if [ "$(cat /etc/samba3.patch.version)" = "$VERSION" ]; then
		echo "ERROR: Changes have been applied!"
		exit 2
	fi
fi

# Verifica versao pfSense
if [ "$(cat /etc/version)" != "2.3.2-RELEASE" ]; then
	echo "ERROR: You need the pfSense version 2.3.2 to apply this script"
	exit 2
fi

arch="`uname -p`"

ASSUME_ALWAYS_YES=YES
export ASSUME_ALWAYS_YES

/usr/sbin/pkg bootstrap
/usr/sbin/pkg update

# Lock packages necessary
/usr/sbin/pkg lock pkg
/usr/sbin/pkg lock pfSense-2.3.2

mkdir -p /usr/local/etc/pkg/repos

cat <<EOF > /usr/local/etc/pkg/repos/pf2ad.conf
pf2ad: {
    url: "http://projetos.mundounix.com.br/pfsense/2.3/packages/${arch}",
    mirror_type: "http",
    enabled: yes
}
EOF

/usr/sbin/pkg update -r pf2ad
/usr/sbin/pkg install -r pf2ad net/samba36 2> /dev/null

/usr/sbin/pkg unlock pkg
/usr/sbin/pkg unlock pfSense-2.3.2
/usr/sbin/pkg unlock dnsmasq-devel

rm -rf /usr/local/etc/pkg/repos/pf2ad.conf
/usr/sbin/pkg update

mkdir -p /var/db/samba/winbindd_privileged
chown -R :proxy /var/db/samba/winbindd_privileged
chmod -R 0750 /var/db/samba/winbindd_privileged

fetch -o /usr/local/pkg -q http://projetos.mundounix.com.br/pfsense/2.3/samba3/samba3.inc
fetch -o /usr/local/pkg -q http://projetos.mundounix.com.br/pfsense/2.3/samba3/samba3.xml

/usr/local/sbin/pfSsh.php <<EOF
\$samba3 = false;
foreach (\$config['installedpackages']['service'] as \$item) {
  if ('samab3' == \$item['name']) {
    \$samba3 = true;
    break;
  }
}
if (\$samba3 == false) {
	\$config['installedpackages']['service'][] = array(
	  'name' => 'samba3',
	  'rcfile' => 'samba3.sh',
	  'executable' => 'smbd',
	  'description' => 'Samba 3 daemon'
  );
}
\$samba3 = false;
foreach (\$config['installedpackages']['menu'] as \$item) {
  if ('Samba3 (AD)' == \$item['name']) {
    \$samba3 = true;
    break;
  }
}
if (\$samba3 == false) {
  \$config['installedpackages']['menu'][] = array(
    'name' => 'Samba3 (AD)',
    'section' => 'Services',
    'url' => '/pkg_edit.php?xml=samba3.xml'
  );
}
write_config();
exec;
exit
EOF

if [ ! -f "/usr/bin/install" ]; then
	fetch -o /usr/bin/install -q http://projetos.mundounix.com.br/pfsense/bin/install-${arch}
	chmod +x /usr/bin/install
fi

if [ ! "$(/usr/sbin/pkg info | grep pfSense-pkg-squid)" ]; then
	/usr/sbin/pkg install -r pfSense pfSense-pkg-squid
fi

cd /usr/local/pkg
if ! fetch -o - -q http://projetos.mundounix.com.br/pfsense/2.3/samba3/squid_ntlm.patch | patch -p0 --dry-run -t | grep "Reversed"; then
    fetch -o - -q http://projetos.mundounix.com.br/pfsense/2.3/samba3/squid_ntlm.patch | patch -b -p0
fi

if [ ! -f "/usr/local/etc/smb.conf" ]; then
	touch /usr/local/etc/smb.conf
fi
cp -f /usr/local/bin/ntlm_auth /usr/local/libexec/squid/ntlm_auth

/etc/rc.d/ldconfig restart

echo "$VERSION" > /etc/samba3.patch.version
