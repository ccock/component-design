Name:		hello
Version:	1.0.0
Release:	1%{?dist}
Summary:	hello static lib for test rpm

Group:		wisdomcoda
License:	GPLv2
URL:		file://home/root/rpm/hello
Source0:	hello-1.0.0.tar.gz

BuildRequires:	gcc,make
Requires:	httpd

%description


%prep
%setup -q


%build
make all


%install
make install DESTDIR=%{buildroot}


%files
/libhello.a

%doc



%changelog

