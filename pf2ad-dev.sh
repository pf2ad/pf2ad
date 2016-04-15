#!/bin/sh

VERSION='20150916023'

if [ -f "/etc/samba3.patch.version" ]; then
	if [ "$(cat /etc/samba3.patch.version)" = "$VERSION" ]; then
		echo "ERROR: Changes have been applied!"
		exit 2
	fi
fi

# Verifica versao pfSense
if [ "$(cat /etc/version)" != "2.2.4-RELEASE" ]; then
	echo "ERROR: You need the pfSense version 2.2.4 to apply this script"
	exit 2
fi

arch="`uname -p`"

ASSUME_ALWAYS_YES=YES
export ASSUME_ALWAYS_YES

/usr/sbin/pkg bootstrap
/usr/sbin/pkg update

mkdir -p /usr/local/etc/pkg/repos

cat <<EOF > /usr/local/etc/pkg/repos/samba36.conf
samba36: {
    url: "http://projetos.mundounix.com.br/pfsense/packages/samba36/${arch}",
    mirror_type: "http",
    enabled: yes
}
EOF

/usr/sbin/pkg update -r samba36
/usr/sbin/pkg install -r samba36 net/samba36

echo 'samba_enable="YES"' > /etc/rc.conf.d/samba
echo 'winbindd_enable="YES"' > /etc/rc.conf.d/winbindd

cd /usr/local/etc/rc.d/
if [ ! -f "samba.sh" ]; then
	ln -s samba samba.sh
fi

mkdir -p /var/db/samba/winbindd_privileged
chown -R :proxy /var/db/samba/winbindd_privileged
chmod -R 0750 /var/db/samba/winbindd_privileged

fetch -o /usr/local/pkg -q http://projetos.mundounix.com.br/pfsense/2.2.4/samba3/samba3.inc
fetch -o /usr/local/pkg -q http://projetos.mundounix.com.br/pfsense/2.2.4/samba3/samba3.xml
fetch -o /usr/local/www/javascript -q http://projetos.mundounix.com.br/pfsense/2.2.4/samba3/jquery-1.9.1.min.js

cd /usr/local/www
if ! fetch -o - -q http://projetos.mundounix.com.br/pfsense/2.2.4/samba3/fbegin.inc.patch | patch -p0 --dry-run -t | grep "Reversed"; then
	fetch -o - -q http://projetos.mundounix.com.br/pfsense/2.2.4/samba3/fbegin.inc.patch | patch -b -p0
fi

if [ ! "$(/usr/local/sbin/pfSsh.php playback listpkg | grep 'squid3')" ]; then
	/usr/local/sbin/pfSsh.php playback installpkg "squid3"
fi
if [ ! "$(/usr/local/sbin/pfSsh.php playback listpkg | grep 'squidGuard-devel')" ]; then
    mkdir -p /usr/pbi/squidguard-devel-${arch}/etc/squidGuard
    touch /usr/pbi/squidguard-devel-${arch}/etc/squidGuard/squidguard_conf.xml
	/usr/local/sbin/pfSsh.php playback installpkg "squidGuard-devel"
fi

if [ "$(pkg info | grep db5-5)" ]; then
	pkg delete db5
fi

if [ -f "/usr/local/bin/squidGuard" ]; then
	if [ "$(md5 -q /usr/local/bin/squidGuard)" != "f889ffd71c25926d46cebd839e8ea117" ]; then
		pkg install db48
		rm -rf /usr/local/bin/squidGuard
		fetch -q -o /usr/local/bin/squidGuard http://projetos.mundounix.com.br/pfsense/2.2.4/samba3/squidGuard-${arch}
		chmod +x /usr/local/bin/squidGuard
	fi
fi

if [ "$(md5 -q /usr/pbi/squidguard-devel-${arch}/bin/squidGuard)" != "f889ffd71c25926d46cebd839e8ea117" ]; then
	cp -f /usr/local/bin/squidGuard /usr/pbi/squidguard-devel-${arch}/bin/
	chmod +x /usr/pbi/squidguard-devel-${arch}/bin/squidGuard
fi

# apply patch to fix use quotes
cd /usr/local/pkg
if ! fetch -o - -q http://projetos.mundounix.com.br/pfsense/2.2.4/samba3/squidguard-quote.patch | patch -p0 --dry-run -t | grep "Reversed"; then
    fetch -o - -q http://projetos.mundounix.com.br/pfsense/2.2.4/samba3/squidguard-quote.patch | patch -b -p0
fi

# Fix libs in pbi
cp -r /usr/local/lib/libdb* /usr/local/lib/db48 /usr/pbi/squidguard-devel-${arch}/local/lib/
cp -r /usr/local/lib/libdb* /usr/local/lib/db48 /usr/pbi/squid-${arch}/local/lib/

cd /usr/local/pkg
if ! fetch -o - -q http://projetos.mundounix.com.br/pfsense/2.2.4/samba3/squid3_with_ntlm.patch | patch -p0 --dry-run -t | grep "Reversed"; then
	fetch -o - -q http://projetos.mundounix.com.br/pfsense/2.2.4/samba3/squid3_with_ntlm.patch | patch -b -p0
fi

if [ ! -f "/usr/local/etc/smb.conf" ]; then
	touch /usr/local/etc/smb.conf
fi
if [ ! -f "/usr/pbi/squid-${arch}/local/etc/smb.conf" ]; then
	ln /usr/local/etc/smb.conf /usr/pbi/squid-${arch}/local/etc/smb.conf
fi
if [ -f "/usr/pbi/squid-${arch}/local/libexec/squid/ntlm_auth" ]; then
    if [ "$(md5 -q /usr/pbi/squid-${arch}/local/libexec/squid/ntlm_auth)" != "7d0dec78872956dada2057fee81031aa" ]; then
	    cp -f /usr/local/bin/ntlm_auth /usr/pbi/squid-${arch}/local/libexec/squid/ntlm_auth
    fi
else
    cp -f /usr/local/bin/ntlm_auth /usr/pbi/squid-${arch}/local/libexec/squid/ntlm_auth
fi

/etc/rc.d/ldconfig restart

echo "$VERSION" > /etc/samba3.patch.version
