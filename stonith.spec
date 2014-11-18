Summary: Suicide driver
Name: redgate-stonith
Version: 1.0
Release: 100
Group: System Environment/Tools
License: GPL
Vendor: Redgate
Packager: Karl N. Redgate <Karl.Redgate@gmail.com>
BuildRoot: %(echo $PWD)/exports
%define _topdir %(echo $PWD)/rpm

%description
Driver and command line tool to provide STONITH device in kernel.

%prep
%build
%install
%files
%defattr(-,root,root,0755)
/usr/sbin/shoot
/lib/modules/.../devices/misc/stonith.ko

%post
[ "$1" -gt 1 ] && {
    echo "`date '+%%b %%e %%H:%%M:%%S'`: Upgrading stonith"
}

[ "$1" = 1 ] && {
    echo "`date '+%%b %%e %%H:%%M:%%S'`: New install of stonith"
}

[ "$1" -gt 1 ] && {
    echo "`date '+%%b %%e %%H:%%M:%%S'`: upgrading"
}
echo "`date '+%%b %%e %%H:%%M:%%S'`: Done"

%changelog
* Mon Sep  5 2011 Karl N. Redgate <Karl.Redgate@gmail.com>
- First package
