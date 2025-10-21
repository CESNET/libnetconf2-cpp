/*
 * Copyright (C) 2021 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Václav Kubernát <kubernat@cesnet.cz>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/
#include <doctest/doctest.h>
#include <fstream>
#include <filesystem>
#include <sstream>
#include "mock_server.hpp"
#include "test_vars.hpp"

using namespace std::string_literals;

namespace mock_server  {
namespace {
const auto yangLib = R"(
<data>
    <yang-library xmlns="urn:ietf:params:xml:ns:yang:ietf-yang-library">
        <module-set>
            <name>complete</name>
            <module>
                <name>yang</name>
                <revision>2025-01-29</revision>
                <namespace>urn:ietf:params:xml:ns:yang:1</namespace>
            </module>
            <module>
                <name>ietf-yang-schema-mount</name>
                <revision>2019-01-14</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-schema-mount</namespace>
            </module>
            <module>
                <name>ietf-datastores</name>
                <revision>2018-02-14</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-datastores</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-datastores@2018-02-14.yang</location>
            </module>
            <module>
                <name>ietf-yang-library</name>
                <revision>2019-01-04</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-yang-library@2019-01-04.yang</location>
            </module>
            <module>
                <name>ietf-netconf-acm</name>
                <revision>2018-02-14</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-acm</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-netconf-acm@2018-02-14.yang</location>
            </module>
            <module>
                <name>ietf-netconf</name>
                <revision>2013-09-29</revision>
                <namespace>urn:ietf:params:xml:ns:netconf:base:1.0</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-netconf@2013-09-29.yang</location>
                <feature>writable-running</feature>
                <feature>candidate</feature>
                <feature>confirmed-commit</feature>
                <feature>rollback-on-error</feature>
                <feature>validate</feature>
                <feature>startup</feature>
                <feature>url</feature>
                <feature>xpath</feature>
            </module>
            <module>
                <name>ietf-netconf-with-defaults</name>
                <revision>2011-06-01</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults</namespace>
                <location>
                    file:///opt/cesnet-au/sysrepo/repository/yang/ietf-netconf-with-defaults@2011-06-01.yang
                </location>
            </module>
            <module>
                <name>ietf-netconf-notifications</name>
                <revision>2012-02-06</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-notifications</namespace>
                <location>
                    file:///opt/cesnet-au/sysrepo/repository/yang/ietf-netconf-notifications@2012-02-06.yang
                </location>
            </module>
            <module>
                <name>ietf-origin</name>
                <revision>2018-02-14</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-origin</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-origin@2018-02-14.yang</location>
            </module>
            <module>
                <name>ietf-netconf-monitoring</name>
                <revision>2010-10-04</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring</namespace>
                <location>
                    file:///opt/cesnet-au/sysrepo/repository/yang/ietf-netconf-monitoring@2010-10-04.yang
                </location>
            </module>
            <module>
                <name>ietf-netconf-nmda</name>
                <revision>2019-01-07</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-nmda</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-netconf-nmda@2019-01-07.yang</location>
                <feature>origin</feature>
                <feature>with-defaults</feature>
            </module>
            <module>
                <name>nc-notifications</name>
                <revision>2008-07-14</revision>
                <namespace>urn:ietf:params:xml:ns:netmod:notification</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/nc-notifications@2008-07-14.yang</location>
            </module>
            <module>
                <name>notifications</name>
                <revision>2008-07-14</revision>
                <namespace>urn:ietf:params:xml:ns:netconf:notification:1.0</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/notifications@2008-07-14.yang</location>
            </module>
            <module>
                <name>ietf-x509-cert-to-name</name>
                <revision>2014-12-10</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-x509-cert-to-name</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-x509-cert-to-name@2014-12-10.yang</location>
            </module>
            <module>
                <name>ietf-keystore</name>
                <revision>2019-07-02</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-keystore</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-keystore@2019-07-02.yang</location>
                <feature>keystore-supported</feature>
            </module>
            <module>
                <name>ietf-truststore</name>
                <revision>2019-07-02</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-truststore</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-truststore@2019-07-02.yang</location>
                <feature>truststore-supported</feature>
                <feature>x509-certificates</feature>
            </module>
            <module>
                <name>ietf-tcp-common</name>
                <revision>2019-07-02</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-tcp-common</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-tcp-common@2019-07-02.yang</location>
                <feature>keepalives-supported</feature>
            </module>
            <module>
                <name>ietf-ssh-server</name>
                <revision>2019-07-02</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-ssh-server</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-ssh-server@2019-07-02.yang</location>
                <feature>local-client-auth-supported</feature>
            </module>
            <module>
                <name>ietf-tls-server</name>
                <revision>2019-07-02</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-tls-server</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-tls-server@2019-07-02.yang</location>
                <feature>local-client-auth-supported</feature>
            </module>
            <module>
                <name>ietf-netconf-server</name>
                <revision>2019-07-02</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-server</namespace>
                <location>
                    file:///opt/cesnet-au/sysrepo/repository/yang/ietf-netconf-server@2019-07-02.yang
                </location>
                <feature>ssh-listen</feature>
                <feature>tls-listen</feature>
                <feature>ssh-call-home</feature>
                <feature>tls-call-home</feature>
            </module>
            <module>
                <name>ietf-interfaces</name>
                <revision>2018-02-20</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-interfaces</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-interfaces@2018-02-20.yang</location>
            </module>
            <module>
                <name>ietf-ip</name>
                <revision>2018-02-22</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-ip</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-ip@2018-02-22.yang</location>
            </module>
            <module>
                <name>ietf-network-instance</name>
                <revision>2019-01-21</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-network-instance</namespace>
                <location>
                    file:///opt/cesnet-au/sysrepo/repository/yang/ietf-network-instance@2019-01-21.yang
                </location>
            </module>
            <module>
                <name>ietf-subscribed-notifications</name>
                <revision>2019-09-09</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-subscribed-notifications</namespace>
                <location>
                    file:///opt/cesnet-au/sysrepo/repository/yang/ietf-subscribed-notifications@2019-09-09.yang
                </location>
                <feature>encode-xml</feature>
                <feature>replay</feature>
                <feature>subtree</feature>
                <feature>xpath</feature>
            </module>
            <module>
                <name>ietf-yang-push</name>
                <revision>2019-09-09</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-push</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-yang-push@2019-09-09.yang</location>
                <feature>on-change</feature>
            </module>
            <module>
                <name>example-schema</name>
                <namespace>http://example.com</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/example-schema.yang</location>
            </module>
            <import-only-module>
                <name>ietf-yang-metadata</name>
                <revision>2016-08-05</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-metadata</namespace>
            </import-only-module>
            <import-only-module>
                <name>ietf-inet-types</name>
                <revision>2013-07-15</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-inet-types</namespace>
            </import-only-module>
            <import-only-module>
                <name>ietf-yang-types</name>
                <revision>2013-07-15</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-types</namespace>
            </import-only-module>
            <import-only-module>
                <name>ietf-yang-structure-ext</name>
                <revision>2020-06-17</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-structure-ext</namespace>
            </import-only-module>
            <import-only-module>
                <name>ietf-crypto-types</name>
                <revision>2019-07-02</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-crypto-types</namespace>
                <location>file:///home/jkt/work/prog/_build/czechlight-clang14-asan-ubsan-ly2/target/etc-sysrepo/yang/ietf-crypto-types@2019-07-02.yang</location>
            </import-only-module>
            <import-only-module>
                <name>ietf-ssh-common</name>
                <revision>2019-07-02</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-ssh-common</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-ssh-common@2019-07-02.yang</location>
            </import-only-module>
            <import-only-module>
                <name>iana-crypt-hash</name>
                <revision>2014-08-06</revision>
                <namespace>urn:ietf:params:xml:ns:yang:iana-crypt-hash</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/iana-crypt-hash@2014-08-06.yang</location>
            </import-only-module>
            <import-only-module>
                <name>ietf-tls-common</name>
                <revision>2019-07-02</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-tls-common</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-tls-common@2019-07-02.yang</location>
            </import-only-module>
            <import-only-module>
                <name>ietf-tcp-client</name>
                <revision>2019-07-02</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-tcp-client</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-tcp-client@2019-07-02.yang</location>
            </import-only-module>
            <import-only-module>
                <name>ietf-tcp-server</name>
                <revision>2019-07-02</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-tcp-server</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-tcp-server@2019-07-02.yang</location>
            </import-only-module>
            <import-only-module>
                <name>ietf-restconf</name>
                <revision>2017-01-26</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-restconf</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-restconf@2017-01-26.yang</location>
            </import-only-module>
            <import-only-module>
                <name>ietf-yang-patch</name>
                <revision>2017-02-22</revision>
                <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-patch</namespace>
                <location>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-yang-patch@2017-02-22.yang</location>
            </import-only-module>
        </module-set>
        <schema>
            <name>complete</name>
            <module-set>complete</module-set>
        </schema>
        <datastore>
            <name xmlns:ds="urn:ietf:params:xml:ns:yang:ietf-datastores">
                ds:running
            </name>
            <schema>complete</schema>
        </datastore>
        <datastore>
            <name xmlns:ds="urn:ietf:params:xml:ns:yang:ietf-datastores">
                ds:candidate
            </name>
            <schema>complete</schema>
        </datastore>
        <datastore>
            <name xmlns:ds="urn:ietf:params:xml:ns:yang:ietf-datastores">
                ds:startup
            </name>
            <schema>complete</schema>
        </datastore>
        <datastore>
            <name xmlns:ds="urn:ietf:params:xml:ns:yang:ietf-datastores">
                ds:operational
            </name>
            <schema>complete</schema>
        </datastore>
        <content-id>26</content-id>
    </yang-library>
    <modules-state xmlns="urn:ietf:params:xml:ns:yang:ietf-yang-library">
        <module-set-id>26</module-set-id>
        <module>
            <name>ietf-yang-metadata</name>
            <revision>2016-08-05</revision>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-metadata</namespace>
            <conformance-type>import</conformance-type>
        </module>
        <module>
            <name>yang</name>
            <revision>2025-01-29</revision>
            <namespace>urn:ietf:params:xml:ns:yang:1</namespace>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-inet-types</name>
            <revision>2013-07-15</revision>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-inet-types</namespace>
            <conformance-type>import</conformance-type>
        </module>
        <module>
            <name>ietf-yang-types</name>
            <revision>2013-07-15</revision>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-types</namespace>
            <conformance-type>import</conformance-type>
        </module>
        <module>
            <name>ietf-yang-schema-mount</name>
            <revision>2019-01-14</revision>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-schema-mount</namespace>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-yang-structure-ext</name>
            <revision>2020-06-17</revision>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-structure-ext</namespace>
            <conformance-type>import</conformance-type>
        </module>
        <module>
            <name>ietf-datastores</name>
            <revision>2018-02-14</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-datastores@2018-02-14.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-datastores</namespace>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-yang-library</name>
            <revision>2019-01-04</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-yang-library@2019-01-04.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-netconf-acm</name>
            <revision>2018-02-14</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-netconf-acm@2018-02-14.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-acm</namespace>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-netconf</name>
            <revision>2013-09-29</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-netconf@2013-09-29.yang</schema>
            <namespace>urn:ietf:params:xml:ns:netconf:base:1.0</namespace>
            <feature>writable-running</feature>
            <feature>candidate</feature>
            <feature>confirmed-commit</feature>
            <feature>rollback-on-error</feature>
            <feature>validate</feature>
            <feature>startup</feature>
            <feature>url</feature>
            <feature>xpath</feature>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-netconf-with-defaults</name>
            <revision>2011-06-01</revision>
            <schema>
                file:///opt/cesnet-au/sysrepo/repository/yang/ietf-netconf-with-defaults@2011-06-01.yang
            </schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults</namespace>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-netconf-notifications</name>
            <revision>2012-02-06</revision>
            <schema>
                file:///opt/cesnet-au/sysrepo/repository/yang/ietf-netconf-notifications@2012-02-06.yang
            </schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-notifications</namespace>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-origin</name>
            <revision>2018-02-14</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-origin@2018-02-14.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-origin</namespace>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-netconf-monitoring</name>
            <revision>2010-10-04</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-netconf-monitoring@2010-10-04.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring</namespace>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-netconf-nmda</name>
            <revision>2019-01-07</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-netconf-nmda@2019-01-07.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-nmda</namespace>
            <feature>origin</feature>
            <feature>with-defaults</feature>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>nc-notifications</name>
            <revision>2008-07-14</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/nc-notifications@2008-07-14.yang</schema>
            <namespace>urn:ietf:params:xml:ns:netmod:notification</namespace>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>notifications</name>
            <revision>2008-07-14</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/notifications@2008-07-14.yang</schema>
            <namespace>urn:ietf:params:xml:ns:netconf:notification:1.0</namespace>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-x509-cert-to-name</name>
            <revision>2014-12-10</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-x509-cert-to-name@2014-12-10.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-x509-cert-to-name</namespace>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-keystore</name>
            <revision>2019-07-02</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-keystore@2019-07-02.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-keystore</namespace>
            <feature>keystore-supported</feature>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-crypto-types</name>
            <revision>2019-07-02</revision>
            <schema>file:///home/jkt/work/prog/_build/czechlight-clang14-asan-ubsan-ly2/target/etc-sysrepo/yang/ietf-crypto-types@2019-07-02.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-crypto-types</namespace>
            <conformance-type>import</conformance-type>
        </module>
        <module>
            <name>ietf-truststore</name>
            <revision>2019-07-02</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-truststore@2019-07-02.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-truststore</namespace>
            <feature>truststore-supported</feature>
            <feature>x509-certificates</feature>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-tcp-common</name>
            <revision>2019-07-02</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-tcp-common@2019-07-02.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-tcp-common</namespace>
            <feature>keepalives-supported</feature>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-ssh-server</name>
            <revision>2019-07-02</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-ssh-server@2019-07-02.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-ssh-server</namespace>
            <feature>local-client-auth-supported</feature>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-ssh-common</name>
            <revision>2019-07-02</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-ssh-common@2019-07-02.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-ssh-common</namespace>
            <conformance-type>import</conformance-type>
        </module>
        <module>
            <name>iana-crypt-hash</name>
            <revision>2014-08-06</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/iana-crypt-hash@2014-08-06.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:iana-crypt-hash</namespace>
            <conformance-type>import</conformance-type>
        </module>
        <module>
            <name>ietf-tls-server</name>
            <revision>2019-07-02</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-tls-server@2019-07-02.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-tls-server</namespace>
            <feature>local-client-auth-supported</feature>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-tls-common</name>
            <revision>2019-07-02</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-tls-common@2019-07-02.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-tls-common</namespace>
            <conformance-type>import</conformance-type>
        </module>
        <module>
            <name>ietf-netconf-server</name>
            <revision>2019-07-02</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-netconf-server@2019-07-02.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-server</namespace>
            <feature>ssh-listen</feature>
            <feature>tls-listen</feature>
            <feature>ssh-call-home</feature>
            <feature>tls-call-home</feature>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-tcp-client</name>
            <revision>2019-07-02</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-tcp-client@2019-07-02.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-tcp-client</namespace>
            <conformance-type>import</conformance-type>
        </module>
        <module>
            <name>ietf-tcp-server</name>
            <revision>2019-07-02</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-tcp-server@2019-07-02.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-tcp-server</namespace>
            <conformance-type>import</conformance-type>
        </module>
        <module>
            <name>ietf-interfaces</name>
            <revision>2018-02-20</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-interfaces@2018-02-20.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-interfaces</namespace>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-ip</name>
            <revision>2018-02-22</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-ip@2018-02-22.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-ip</namespace>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-network-instance</name>
            <revision>2019-01-21</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-network-instance@2019-01-21.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-network-instance</namespace>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-subscribed-notifications</name>
            <revision>2019-09-09</revision>
            <schema>
                file:///opt/cesnet-au/sysrepo/repository/yang/ietf-subscribed-notifications@2019-09-09.yang
            </schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-subscribed-notifications</namespace>
            <feature>encode-xml</feature>
            <feature>replay</feature>
            <feature>subtree</feature>
            <feature>xpath</feature>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-restconf</name>
            <revision>2017-01-26</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-restconf@2017-01-26.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-restconf</namespace>
            <conformance-type>import</conformance-type>
        </module>
        <module>
            <name>ietf-yang-push</name>
            <revision>2019-09-09</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-yang-push@2019-09-09.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-push</namespace>
            <feature>on-change</feature>
            <conformance-type>implement</conformance-type>
        </module>
        <module>
            <name>ietf-yang-patch</name>
            <revision>2017-02-22</revision>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/ietf-yang-patch@2017-02-22.yang</schema>
            <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-patch</namespace>
            <conformance-type>import</conformance-type>
        </module>
        <module>
            <name>example-schema</name>
            <revision/>
            <schema>file:///opt/cesnet-au/sysrepo/repository/yang/example-schema.yang</schema>
            <namespace>http://example.com</namespace>
            <conformance-type>implement</conformance-type>
        </module>
    </modules-state>
</data>
)";

// FIXME: This is copied from what the Netopeer2 server sends. Some capabilities were removed.
// I better make sure to check which of these are actually needed.
const auto serverHello = R"(
<hello xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <capabilities>
        <capability>urn:ietf:params:netconf:base:1.0</capability>
        <capability>urn:ietf:params:netconf:base:1.1</capability>
        <capability>urn:ietf:params:netconf:capability:writable-running:1.0</capability>
        <capability>urn:ietf:params:netconf:capability:candidate:1.0</capability>
        <capability>urn:ietf:params:netconf:capability:confirmed-commit:1.1</capability>
        <capability>urn:ietf:params:netconf:capability:rollback-on-error:1.0</capability>
        <capability>urn:ietf:params:netconf:capability:validate:1.1</capability>
        <capability>urn:ietf:params:netconf:capability:startup:1.0</capability>
        <capability>urn:ietf:params:netconf:capability:xpath:1.0</capability>
        <capability>
            urn:ietf:params:netconf:capability:with-defaults:1.0?basic-mode=explicit&amp;also-supported=report-all,report-all-tagged,trim,explicit
        </capability>
        <capability>urn:ietf:params:netconf:capability:notification:1.0</capability>
        <capability>urn:ietf:params:netconf:capability:interleave:1.0</capability>
        <capability>urn:ietf:params:netconf:capability:url:1.0?scheme=scp,http,https,ftp,sftp,ftps,file</capability>
        <capability>
            urn:ietf:params:xml:ns:yang:ietf-yang-metadata?module=ietf-yang-metadata&amp;revision=2016-08-05
        </capability>
        <capability>urn:ietf:params:xml:ns:yang:1?module=yang&amp;revision=2025-01-29</capability>
        <capability>
            urn:ietf:params:xml:ns:yang:ietf-inet-types?module=ietf-inet-types&amp;revision=2013-07-15
        </capability>
        <capability>
            urn:ietf:params:xml:ns:yang:ietf-yang-types?module=ietf-yang-types&amp;revision=2013-07-15
        </capability>
        <capability>
            urn:ietf:params:netconf:capability:yang-library:1.1?revision=2019-01-04&amp;content-id=27
        </capability>
        <capability>urn:ietf:params:xml:ns:yang:ietf-netconf-acm?module=ietf-netconf-acm&amp;revision=2018-02-14</capability>
        <capability>
            urn:ietf:params:xml:ns:netconf:base:1.0?module=ietf-netconf&amp;revision=2013-09-29&amp;features=writable-running,candidate,confirmed-commit,rollback-on-error,validate,startup,url,xpath
        </capability>
        <capability>
            urn:ietf:params:xml:ns:yang:ietf-netconf-acm?module=ietf-netconf-acm&amp;revision=2018-02-14
        </capability>
        <capability>
            urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults?module=ietf-netconf-with-defaults&amp;revision=2011-06-01
        </capability>
        <capability>
            urn:ietf:params:xml:ns:yang:ietf-netconf-notifications?module=ietf-netconf-notifications&amp;revision=2012-02-06
        </capability>
        <capability>
            urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring?module=ietf-netconf-monitoring&amp;revision=2010-10-04
        </capability>
        <capability>urn:ietf:params:xml:ns:netmod:notification?module=nc-notifications&amp;revision=2008-07-14</capability>
        <capability>urn:ietf:params:xml:ns:netconf:notification:1.0?module=notifications&amp;revision=2008-07-14</capability>
        <capability>urn:ietf:params:xml:ns:yang:ietf-x509-cert-to-name?module=ietf-x509-cert-to-name&amp;revision=2014-12-10</capability>
        <capability>urn:ietf:params:xml:ns:yang:iana-crypt-hash?module=iana-crypt-hash&amp;revision=2014-08-06</capability>
    </capabilities>
    <session-id>1</session-id>
</hello>
]]>]]>
)";

std::string escapeXMLchars(const std::string& input)
{
    std::ostringstream oss;
    for (const auto& ch : input) {

        switch (ch) {
        case '"':
            oss << "&quot;";
            break;
        case '\'':
            oss << "&apos;";
           break;
        case '<':
            oss << "&lt;";
            break;
        case '>':
            oss << "&gt;";
            break;
        case '&':
            oss << "&amp;";
            break;
        default:
            oss << ch;
        }
    }

    return oss.str();
}

void sendMsgWithSize(boost::process::opstream& processInput, const std::string& msg)
{
    processInput << "\n#" << msg.size() << "\n" << msg << "\n\n##\n";
    processInput.flush();
}

std::string readFileToString(const std::filesystem::path& path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open())
        throw std::invalid_argument("File '" + std::string(path) + "' does not exist.");

    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

void sendHello(boost::process::opstream& processInput)
{
    processInput << serverHello;
    processInput.flush();
}

auto loadModuleStr(const std::string& module_and_revision)
{
    return readFileToString(TESTS_DIR "/modules/" + module_and_revision + ".yang");
}

void sendModule(int msgId, boost::process::opstream& processInput, const std::string& module_and_revision)
{
    const auto moduleMessageStart = R"(<data xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring">)"s;
    const auto moduleMessageEnd = R"(</data>)"s;

    sendRpcReply(msgId, processInput, moduleMessageStart + escapeXMLchars(loadModuleStr(module_and_revision)) + moduleMessageEnd);
}

void skipClientHello(boost::process::ipstream& processOutput)
{
    std::string msg;
    while (!msg.ends_with("]]>]]>")) {
        msg.push_back(char(processOutput.get()));
    }
}
}

void sendRpcReply(int msgId, boost::process::opstream& processInput, std::string data)
{
    const auto rpcReplyStartTag = R"(<rpc-reply xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" message-id=")" + std::to_string(msgId) + R"(">)"s;
    const auto rpcReplyEndTag = R"(</rpc-reply>)"s;
    sendMsgWithSize(processInput, (rpcReplyStartTag + data + rpcReplyEndTag));
}

void skipNetconfChunk(boost::process::ipstream& processOutput, const std::vector<std::string>& mustContain)
{
    REQUIRE(processOutput.get() == '\n');
    REQUIRE(processOutput.get() == '#');
    int size;
    processOutput >> size;
    auto buf = std::make_unique<char[]>(size + 1);
    processOutput.read(buf.get(), size + 1);
    REQUIRE(processOutput.get() == '\n');
    REQUIRE(processOutput.get() == '#');
    REQUIRE(processOutput.get() == '#');
    REQUIRE(processOutput.get() == '\n');
    auto view = std::string_view(buf.get(), size + 1);
    for (const auto& str : mustContain) {
        CAPTURE(str);
        CAPTURE(view);
        REQUIRE(view.find(str) != std::string_view::npos);
    }
}

enum class Latest {
    Yes,
    No
};

void handleSessionStart(int& curMsgId, boost::process::opstream& processInput, boost::process::ipstream& processOutput)
{
    auto resolveGetSchema = [&] (const auto modName, const char* revision, const auto latest) {
        const auto expectedRpc = R"(<rpc xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" message-id=")" +
            std::to_string(curMsgId) +
            R"("><get-schema xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring"><identifier>)" +
            modName +
            R"(</identifier>)" +
            (latest == Latest::Yes ? "" : R"(<version>)"s + revision + R"(</version>)") +
            R"(<format>yang</format></get-schema></rpc>)";
        skipNetconfChunk(processOutput, {expectedRpc});
        sendModule(curMsgId++, processInput, modName + (revision ? ("@"s + revision) : ""));
    };
    skipClientHello(processOutput);
    sendHello(processInput);
    resolveGetSchema("ietf-netconf", "2013-09-29", Latest::Yes);
    resolveGetSchema("ietf-netconf-acm", "2018-02-14", Latest::No);
    skipNetconfChunk(processOutput, {});
    sendRpcReply(curMsgId++, processInput, yangLib);
    resolveGetSchema("ietf-netconf-nmda", "2019-01-07", Latest::Yes);
    resolveGetSchema("ietf-origin", "2018-02-14", Latest::No);
    resolveGetSchema("ietf-netconf-with-defaults", "2011-06-01", Latest::No);
    resolveGetSchema("ietf-netconf-notifications", "2012-02-06", Latest::No);
    resolveGetSchema("nc-notifications", "2008-07-14", Latest::No);
    resolveGetSchema("notifications", "2008-07-14", Latest::No);
    resolveGetSchema("ietf-x509-cert-to-name", "2014-12-10", Latest::No);
    resolveGetSchema("ietf-keystore", "2019-07-02", Latest::No);
    resolveGetSchema("ietf-crypto-types", "2019-07-02", Latest::No);
    resolveGetSchema("ietf-truststore", "2019-07-02", Latest::No);
    resolveGetSchema("ietf-tcp-common", "2019-07-02", Latest::No);
    resolveGetSchema("ietf-ssh-server", "2019-07-02", Latest::No);
    resolveGetSchema("ietf-ssh-common", "2019-07-02", Latest::No);
    resolveGetSchema("iana-crypt-hash", "2014-08-06", Latest::No);
    resolveGetSchema("ietf-tls-server", "2019-07-02", Latest::No);
    resolveGetSchema("ietf-tls-common", "2019-07-02", Latest::No);
    resolveGetSchema("ietf-netconf-server", "2019-07-02", Latest::No);
    resolveGetSchema("ietf-tcp-client", "2019-07-02", Latest::No);
    resolveGetSchema("ietf-tcp-server", "2019-07-02", Latest::No);
    resolveGetSchema("ietf-interfaces", "2018-02-20", Latest::No);
    resolveGetSchema("ietf-ip", "2018-02-22", Latest::No);
    resolveGetSchema("ietf-network-instance", "2019-01-21", Latest::No);
    resolveGetSchema("ietf-subscribed-notifications", "2019-09-09", Latest::No);
    resolveGetSchema("ietf-restconf", "2017-01-26", Latest::No);
    resolveGetSchema("ietf-yang-push", "2019-09-09", Latest::No);
    resolveGetSchema("ietf-yang-patch", "2017-02-22", Latest::No);
    resolveGetSchema("example-schema", nullptr, Latest::Yes);
}
}
