
```sh
mkdir -p RPMBUILD/{BUILD,BUILDROOT,RPMS,SOURCES,SEPCS,SRPMS}

vim ~/.rpmmacros 

%_topdir       %{getenv:HOME}/rpm/RPM_BUILD
```

```sh
cd ..
tar czvf hello-1.0.0.tar.gz ./hello-1.0.0
cp hello-1.0.0.tar.gz RPM_BUILD/SOURCES
cp hello-1.0.0.spec RPM_BUILD/SPECS
cd RPM_BUILD/SPECS
rpm -ba hello-1.0.0.spec
```

```sh
mkdir yumrepo
cp RPM_BUILD/RPMS/x86_64/*  yumrepo

cd yumrepo
createrepo ./

cd /etc/yum.repos.d/
vim CentOS-Test.repo

[Centos-Test]
name=centos test repo
baseurl=file:///root/rpm/yumrepo
enable=1
gpgcheck=0
priority=1
```

```sh
yum update
yum install hello
```