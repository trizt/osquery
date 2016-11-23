osquery
=======

<p align="center">
<img align="center" src="https://www.gentoo.org/assets/img/logo/gentoo-3d-small.png" alt="Gentoo logo" />
<img align="center" src="https://osquery.io/assets/logo-dark.png" alt="osquery logo" width="200"/>

<p align="center">
osquery is an operating system instrumentation framework for OS X and Linux. <br/>
The tools make low-level operating system analytics and monitoring both performant and intuitive.

| Platform | Build status  | | | |
|----------|---------------|---|---|---|
CentOS 7.x   | [![Build Status](https://jenkins.osquery.io/job/osqueryMasterBuildCentOS7/badge/icon)](https://jenkins.osquery.io/job/osqueryMasterBuildCentOS7/) | | **Packs:** | https://osquery.io/packs
Ubuntu 16.04 | [![Build Status](https://jenkins.osquery.io/job/osqueryMasterBuildUbuntu16/badge/icon)](https://jenkins.osquery.io/job/osqueryMasterBuildUbuntu16/) | | |

Which ever CentOS or Ubuntu version of osquery should work fine on Gentoo, as they by default has support for a few portage related tables.


#### What is osquery?

osquery exposes an operating system as a high-performance relational database. This allows you to write SQL-based queries to explore operating system data. With osquery, SQL tables represent abstract concepts such as running processes, loaded kernel modules, open network connections, browser plugins, hardware events or file hashes.

SQL tables are implemented via a simple plugin and extensions API. A variety of tables already exist and more are being written: [https://osquery.io/tables](https://osquery.io/tables). To best understand the expressiveness that is afforded to you by osquery, consider the following SQL queries:


List the [`users`](https://osquery.io/docs/tables/#users):
```sql
SELECT * FROM users;
```

Check the [`processes`](https://osquery.io/docs/tables/#processes) that have a deleted executable:
```sql
SELECT * FROM processes WHERE on_disk = 0;
```

Get the process name, port, and PID, for processes listening on all interfaces:
```sql
SELECT DISTINCT processes.name, listening_ports.port, processes.pid
  FROM listening_ports JOIN processes USING (pid)
  WHERE listening_ports.address = '0.0.0.0';
```

Find every OS X LaunchDaemon that launches an executable and keeps it running:
```sql
SELECT name, program || program_arguments AS executable
  FROM launchd
  WHERE (run_at_load = 1 AND keep_alive = 1)
  AND (program != '' OR program_arguments != '');
```

Check for ARP anomalies from the host's perspective:

```sql
SELECT address, mac, COUNT(mac) AS mac_count
  FROM arp_cache GROUP BY mac
  HAVING count(mac) > 1;
```

Alternatively, you could also use a SQL sub-query to accomplish the same result:

```sql
SELECT address, mac, mac_count
  FROM
    (SELECT address, mac, COUNT(mac) AS mac_count FROM arp_cache GROUP BY mac)
  WHERE mac_count > 1;
```

These queries can be:
* performed on an ad-hoc basis to explore operating system state using the [osqueryi](https://osquery.readthedocs.org/en/latest/introduction/using-osqueryi/) shell
* executed via a [scheduler](https://osquery.readthedocs.org/en/latest/introduction/using-osqueryd/) to monitor operating system state across a set of hosts
* launched from custom applications using osquery Thrift APIs

#### Gentoo/Funtoo specific tables

```sql
SELECT * FROM portage_keywords;
```

| package                      | version    | keyword | mask | unmask |
|------------------------------|------------|---------|------|--------|
media-libs/opencv            | 3.0.0      |         | 0    | 1
app-text/libwpg              | <0.3.0     |         | 1    | 0
app-emulation/vmware-modules | 308.1.1    | **      | 0    | 1

The portage_keywords will show you the content from /etc/portage/package.keywords, /etc/portage/package.mask and /etc/portage/package.unmask in an unified table, where the column value 1 for mask and unmask tells you that there is a row in one of the files.

```sql
SELECT * FROM portage_packages;
```

| package                                     | version                | slot            | build_time | repository | eapi | size       | world |
|---------------------------------------------|------------------------|-----------------|------------|------------|------|------------|-------|
app-accessibility/at-spi2-atk               | 2.20.1                 | 2               | 1471705141 | gentoo     | 6    | 522130     | 0
app-accessibility/at-spi2-core              | 2.20.2                 | 2               | 1471705086 | gentoo     | 6    | 2167640    | 0
app-admin/apache-tools                      | 2.4.16                 | 0               | 1438541170 | gentoo     | 5    | 474758     | 0
app-admin/cgmanager                         | 0.41                   | 0               | 1459021806 | gentoo     | 6    | 1566567    | 0
app-admin/chroot_safe                       | 1.4                    | 0               | 1403809933 | gentoo     | 4    | 18312      | 1

The portage_packages includes all the installed packages and if the package is listed in the /var/lib/portage/world file. Gives you general infromation about the package as version, slot, when it was built, which eapi it used and the size, based on what has been stored in /var/db/pkg.

```sql
SELECT * FROM portage_use;
```

| package                        | version | use          |
|--------------------------------|---------|--------------|
app-accessibility/at-spi2-atk  | 2.20.1  | abi_x86_32
app-accessibility/at-spi2-atk  | 2.20.1  | abi_x86_64
app-accessibility/at-spi2-core | 2.20.2  | X
app-accessibility/at-spi2-core | 2.20.2  | abi_x86_32
app-accessibility/at-spi2-core | 2.20.2  | abi_x86_64

The portage_use lists the USE flags enabled when a package was built, based on what has been stored in /var/db/pkg.

NOTE: portage_packages and portage_use can be slow at first time you select from them, as they do loop through directories in /var/db/pkg and reads the files it need to fetch data from. Second time, you will be selecting from a cached version, which makes the select time to be a lot better.


#### Downloads / Install

For latest stable and nightly builds for OS X and Linux (deb/rpm), as well as yum and apt repository information visit [https://osquery.io/downloads](https://osquery.io/downloads/). For installation information for FreeBSD, which is supported by the osquery community, see the [wiki](https://osquery.readthedocs.org/en/latest/installation/install-freebsd/).

##### Building from source

[Building](https://osquery.readthedocs.org/en/latest/development/building/) osquery from source is encouraged! Join our developer community by giving us feedback in Github issues or submitting pull requests! There can be some issues to build this on Gentoo/Funtoo as the pip seems to fail on some machines (keep in mind it's not the systems pip but linuxbrews).

#### File Integrity Monitoring (FIM)

osquery provides several [FIM features](http://osquery.readthedocs.org/en/stable/deployment/file-integrity-monitoring/) too! Just as OS concepts are represented in tabular form, the daemon can track OS events and later expose them in a table. Tables like [`file_events`](https://osquery.io/docs/tables/#file_events) or [`yara_events`](https://osquery.io/docs/tables/#yara_events) can be selected to retrieve buffered events.

The configuration allows you to organize files and directories for monitoring. Those sets can be paired with lists of YARA signatures or configured for additional monitoring such as access events.

##### Process and socket auditing

There are several forms of [eventing](http://osquery.readthedocs.org/en/stable/development/pubsub-framework/) in osquery along with file modifications and accesses. These range from disk mounts, network reconfigurations, hardware attach and detaching, and process starting. For a complete set review the table documentation and look for names with the `_events` suffix.

#### Vulnerabilities

Facebook has a [bug bounty](https://www.facebook.com/whitehat/) program that includes osquery. If you find a security vulnerability in osquery, please submit it via the process outlined on that page and do not file a public issue. For more information on finding vulnerabilities in osquery, see a recent blog post about [bug-hunting osquery](https://www.facebook.com/notes/facebook-bug-bounty/bug-hunting-osquery/954850014529225).

#### Learn more

Read the [launch blog post](https://code.facebook.com/posts/844436395567983/introducing-osquery/) for background on the project.
If you're interested in learning more about osquery, visit the [users guide](https://osquery.readthedocs.org/) and browse our RFC-labeled Github issues. Development and usage discussion is happing in the osquery Slack, grab an invite automatically: [https://osquery-slack.herokuapp.com/](https://osquery-slack.herokuapp.com/)!
