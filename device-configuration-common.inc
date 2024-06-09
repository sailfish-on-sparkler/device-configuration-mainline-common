%if 0%{?_obs_build_project:1}
%define _build_flavour %{?qa_stage_name}%{!?qa_stage_name:unknown}
%else
%define _build_flavour unknown
%endif

# needs to match the prjconf in pj:tools
%define _obs_build_count %(echo %{release} | awk -F . '{if (NF >= 3) print $3; else print $1 }')
%define _obs_commit_count %(echo %{release} | awk -F . '{if (NF >= 2) print $2; else print $1 }')

%if "%{_build_flavour}" == "release"
%define _version_appendix %{nil}
%else
%define _version_appendix \ (%{_build_flavour})
%endif

%if 0%{!?configs_common:1}
%define configs_common device-configuration-mainline-common
%endif

Name:       device-configuration-%{rpm_device}
Summary:    Configuration for %{vendor} %{device}
Version:    1
%if 0%{?_obs_build_project:1}
Release:    1
%else
%define rel_date %(date +'%%Y%%m%%d%%H%%M')
Release: %{rel_date}
%endif
License:    GPLv2
Source0:    %{name}-%{version}.tar.bz2

Requires: community-adaptation
BuildRequires: community-adaptation
BuildRequires: ssu-kickstart-configuration
BuildRequires: qt5-qttools-kmap2qmap

%description
%{summary}.

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT

mkdir -p $RPM_BUILD_ROOT/var/lib/environment
cp -rf %{configs_common}/environment/* $RPM_BUILD_ROOT/var/lib/environment

mkdir -p $RPM_BUILD_ROOT/%{_datadir}/ssu/board-mappings.d
cp board-mappings.d/05-%{rpm_device}.ini $RPM_BUILD_ROOT/%{_datadir}/ssu/board-mappings.d
mkdir -p $RPM_BUILD_ROOT/%{_datadir}/ssu/kickstart
cp -af kickstart/* $RPM_BUILD_ROOT/%{_datadir}/ssu/kickstart

mkdir -p $RPM_BUILD_ROOT/%{_datadir}/qt5/keymaps
%{_libdir}/qt5/bin/kmap2qmap hwkeys.kmap $RPM_BUILD_ROOT/%{_datadir}/qt5/keymaps/hwkeys.qmap

%if 0%{?_obs_build_project:1}
# build rnd kickstarts on devel and branch_devel level, release kickstarts on all other levels
%if 0%{?qa_stage_devel:1} || 0%{?qa_stage_branch_devel:1}
  KS_LEVELS=true
%else
  KS_LEVELS=false
%endif
%else
  if ssu s 2>/dev/null | grep -q "Release (rnd): latest (devel)"; then
    KS_LEVELS=true
  else
    KS_LEVELS=false
  fi
%endif

# build variant ks files if name file exist
%if 0%{?variant_name_list:1}
  variants=variant_name.list
  if [ -e $variants ]; then
    grep -E -v '^#|^$' $variants | (
      while read line; do
        %gen_ks "$line"
        sed -i s/@VARIANT_NAME@/$line/g $RPM_BUILD_ROOT/%{_datadir}/kickstarts/*$line*.ks
      done)
  fi
%else
  %gen_ks %{rpm_device}
%endif

mkdir -p $RPM_BUILD_ROOT/etc
cat > $RPM_BUILD_ROOT/etc/hw-release <<EOF
# This file is copied as hw-release (analogous to os-release)
NAME="%{vendor_pretty} %{device_pretty}"
ID=%{rpm_device}
MER_HA_DEVICE=%{rpm_device}
MER_HA_VENDOR=%{rpm_vendor}
VERSION="%{version}.%{_obs_build_count}%{_version_appendix}"
VERSION_ID=%{version}.%{_obs_build_count}
PRETTY_NAME="%{device_pretty} %{version}.%{_obs_build_count}%{_version_appendix}"
SAILFISH_BUILD=%{_obs_build_count}
SAILFISH_FLAVOUR=%{_build_flavour}
HOME_URL="https://sailfishos.org"
EOF

%files
%defattr(-,root,root,-)
/etc/hw-release
/var/lib/environment/*
%{_datadir}/ssu/board-mappings.d/*
%{_datadir}/qt5/keymaps/*


%package kickstart-configuration
Summary:    Kickstart configuration for %{rpm_device}
Requires:   ssu-kickstart-configuration
Requires:   %{name} = %{version}-%{release}

%description kickstart-configuration
%{summary}.

%files kickstart-configuration
%defattr(-,root,root,-)
%{_datadir}/ssu/kickstart/*

%package ssu-kickstarts
Summary:    Kickstarts for %{rpm_device}
Provides:   image-configurations

%description ssu-kickstarts
%{summary}.

%files ssu-kickstarts
%defattr(-,root,root,-)
%{_datadir}/kickstarts/*.ks


%package -n patterns-sailfish-device-configuration-common-%{rpm_device}
Summary:    Jolla configuration common
Requires:   patterns-sailfish-applications
Requires:   patterns-sailfish-ui
Recommends: csd

%description -n patterns-sailfish-device-configuration-common-%{rpm_device}
Pattern with packages for common HW configurations

%files -n patterns-sailfish-device-configuration-common-%{rpm_device}


# vim: ft=spec
