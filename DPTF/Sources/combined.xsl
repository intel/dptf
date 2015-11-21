<!-- begin xsl -->
<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- format_id=3E-58-63-46-F8-F7-45-4A-A8-F7-DE-7E-C6-F7-61-A8 -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/dppm_status">

<table border="1">
  <tr bgcolor="#00AEEF" colspan="2">
    <th colspan="2">Policies</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Index</th>
    <th>Name</th>
  </tr>
  <xsl:for-each select="policies/policy">
    <tr>
      <td align='right'><xsl:value-of select="policy_index" /></td>
      <td><xsl:value-of select="policy_name" /></td>
    </tr>
  </xsl:for-each>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="24">
    <th colspan="24">Participants</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Index</th>
    <th>Name</th>
    <th>Description</th>
    <th>Domain Index</th>
    <th>Domain Name</th>
    <th>Temperature</th>
    <th>Aux0</th>
    <th>Aux1</th>
    <th>Hysteresis</th>
    <th>CRT</th>
    <th>HOT</th>
    <th>CR3</th>
    <th>PSV</th>
    <th>NTT</th>
    <th>AC0</th>
    <th>AC1</th>
    <th>AC2</th>
    <th>AC3</th>
    <th>AC4</th>
    <th>AC5</th>
    <th>AC6</th>
    <th>AC7</th>
    <th>AC8</th>
    <th>AC9</th>
  </tr>
  <xsl:for-each select="participants/participant">
  <xsl:for-each select="domains/domain">
    <tr>
      <xsl:choose>
        <xsl:when test="position()=1">
          <td align='right'><xsl:value-of select="../../index" /></td>
          <td><xsl:value-of select="../../participant_properties/name" /></td>

          <xsl:choose>
            <xsl:when test="domain_controls/active_control/active_control_status">
              <td>
                <xsl:value-of select="../../participant_properties/description" />&#160;
                (<xsl:value-of select="domain_controls/active_control/active_control_status/current_control_id" />)
              </td>
            </xsl:when>
            <xsl:otherwise>
              <td><xsl:value-of select="../../participant_properties/description" /></td>
            </xsl:otherwise>
          </xsl:choose>

        </xsl:when>
        <xsl:otherwise>
          <td colspan="3" bgcolor="#FFFFFF"></td>
        </xsl:otherwise>
      </xsl:choose>

      <td align='right'><xsl:value-of select="index" /></td>
      <td><xsl:value-of select="name" /></td>
      <td align='right'><xsl:value-of select="domain_controls/temperature_control/temperature_status" /></td>

      <xsl:choose>
      <xsl:when test="domain_controls/temperature_control/temperature_thresholds">
        <td align='right'><xsl:value-of select="domain_controls/temperature_control/temperature_thresholds/aux0" /></td>
        <td align='right'><xsl:value-of select="domain_controls/temperature_control/temperature_thresholds/aux1" /></td>
        <td align='right'><xsl:value-of select="domain_controls/temperature_control/temperature_thresholds/hysteresis" /></td>
      </xsl:when>
      <xsl:otherwise>
        <td/>
        <td/>
        <td/>
      </xsl:otherwise>
      </xsl:choose>   
      
      <xsl:choose>
      <xsl:when test="position()=1">
        <td align='right'><xsl:value-of select="../../specific_info/crt" /></td>
        <td align='right'><xsl:value-of select="../../specific_info/hot" /></td>
        <td align='right'><xsl:value-of select="../../specific_info/wrm" /></td>
        <td align='right'><xsl:value-of select="../../specific_info/psv" /></td>
        <td align='right'><xsl:value-of select="../../specific_info/ntt" /></td>
        <td align='right'><xsl:value-of select="../../specific_info/ac0" /></td>
        <td align='right'><xsl:value-of select="../../specific_info/ac1" /></td>
        <td align='right'><xsl:value-of select="../../specific_info/ac2" /></td>
        <td align='right'><xsl:value-of select="../../specific_info/ac3" /></td>
        <td align='right'><xsl:value-of select="../../specific_info/ac4" /></td>
        <td align='right'><xsl:value-of select="../../specific_info/ac5" /></td>
        <td align='right'><xsl:value-of select="../../specific_info/ac6" /></td>
        <td align='right'><xsl:value-of select="../../specific_info/ac7" /></td>
        <td align='right'><xsl:value-of select="../../specific_info/ac8" /></td>
        <td align='right'><xsl:value-of select="../../specific_info/ac9" /></td>
      </xsl:when>
      <xsl:otherwise>
        <td/>
        <td/>
        <td/>
        <td/>
        <td/>
        <td/>
        <td/>
        <td/>
        <td/>
        <td/>
        <td/>
        <td/>
        <td/>
        <td/>
        <td/>
      </xsl:otherwise>
      </xsl:choose>

      </tr>
    </xsl:for-each>
  </xsl:for-each>
</table>

<br></br>

<table border="1">
  <xsl:variable name="col_span">
    <xsl:value-of select="policies/policy_count"/>
  </xsl:variable>
  <tr bgcolor="#00AEEF" colspan="{$col_span+1}">
    <th colspan="{$col_span+1}">Registered Events</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Event Name</th>
    <xsl:for-each select="policy_manager/policy_status">
      <th><xsl:value-of select="policy_name" /></th>
    </xsl:for-each>
  </tr>
  <xsl:for-each select="policy_manager/events/event">
    <tr>
      <td align='left'><xsl:value-of select="event_name" /></td>
      <xsl:variable name="name">
        <xsl:value-of select="event_name"/>
      </xsl:variable>
      <xsl:for-each select="../../../policy_manager/policy_status">
        <xsl:for-each select="event_values/event">
          <xsl:choose>
          <xsl:when test="event_name=$name">
            <xsl:choose>
              <xsl:when test="event_status='true'">
                <td bgcolor="#CCFFCC" align='center'>yes</td>
              </xsl:when>
              <xsl:otherwise>
                <td align='center'>no</td>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:when>
          </xsl:choose>
        </xsl:for-each>
      </xsl:for-each>
    </tr>
  </xsl:for-each>
</table>

</xsl:template>
</xsl:stylesheet>
<!-- end xsl -->

<!-- begin xsl -->
<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- format_id=89-C3-95-3A-B8-E4-29-46-A5-26-C5-2C-88-62-6B-AE -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/active_policy_status">

<table border="1">
  <tr bgcolor="#00AEEF" colspan="5">
    <th colspan="5">Fan Status</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Participant</th>
    <th>Speed %</th>
    <th>Fine Grain</th>
  </tr>
  <xsl:for-each select="fan_status/active_cooling_control">
  <tr>
    <td align='left'><xsl:value-of select="name"/> (<xsl:value-of select="participant_index"/>)</td>
    <td align='right'><xsl:value-of select="speed"/></td>
    <td align='right'><xsl:value-of select="fine_grain"/></td>
  </tr>
  </xsl:for-each>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="16">
    <th colspan="16">Trip Point Status</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Index</th>
    <th>Name</th>
    <th>Aux0</th>
    <th>Temp</th>
    <th>Aux1</th>
    <th>Hysteresis</th>
    <th>AC0</th>
    <th>AC1</th>
    <th>AC2</th>
    <th>AC3</th>
    <th>AC4</th>
    <th>AC5</th>
    <th>AC6</th>
    <th>AC7</th>
    <th>AC8</th>
    <th>AC9</th>
  </tr>
  <xsl:for-each select="active_trip_point_status/participant">
  <tr>
    <td align='right'><xsl:value-of select="index"/></td>
    <td><xsl:value-of select="name"/></td>
    <td align='right'><xsl:value-of select="temperature_thresholds/aux0"/></td>
    <td align='right'><xsl:value-of select="temperature"/></td>
    <td align='right'><xsl:value-of select="temperature_thresholds/aux1"/></td>
    <td align='right'><xsl:value-of select="temperature_thresholds/hysteresis"/></td>
    <td align='right'><xsl:value-of select="specific_info/ac0"/></td>
    <td align='right'><xsl:value-of select="specific_info/ac1"/></td>
    <td align='right'><xsl:value-of select="specific_info/ac2"/></td>
    <td align='right'><xsl:value-of select="specific_info/ac3"/></td>
    <td align='right'><xsl:value-of select="specific_info/ac4"/></td>
    <td align='right'><xsl:value-of select="specific_info/ac5"/></td>
    <td align='right'><xsl:value-of select="specific_info/ac6"/></td>
    <td align='right'><xsl:value-of select="specific_info/ac7"/></td>
    <td align='right'><xsl:value-of select="specific_info/ac8"/></td>
    <td align='right'><xsl:value-of select="specific_info/ac9"/></td>
  </tr>
  </xsl:for-each>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="15">
    <th colspan="15">ART Status</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Target Index</th>
    <th>Target Object ID</th>
    <th>Source Index</th>
    <th>Source Object ID</th>
    <th>Weight</th>
    <th>AC0</th>
    <th>AC1</th>
    <th>AC2</th>
    <th>AC3</th>
    <th>AC4</th>
    <th>AC5</th>
    <th>AC6</th>
    <th>AC7</th>
    <th>AC8</th>
    <th>AC9</th>
  </tr>
  <xsl:for-each select="art/art_entry">
  <tr>
    <td align='right'><xsl:value-of select="target_index"/></td>
    <td><xsl:value-of select="target_acpi_scope"/></td>
    <td align='right'><xsl:value-of select="source_index"/></td>
    <td><xsl:value-of select="source_acpi_scope"/></td>
    <td align='right'><xsl:value-of select="weight"/></td>
    <td align='right'><xsl:value-of select="ac0"/></td>
    <td align='right'><xsl:value-of select="ac1"/></td>
    <td align='right'><xsl:value-of select="ac2"/></td>
    <td align='right'><xsl:value-of select="ac3"/></td>
    <td align='right'><xsl:value-of select="ac4"/></td>
    <td align='right'><xsl:value-of select="ac5"/></td>
    <td align='right'><xsl:value-of select="ac6"/></td>
    <td align='right'><xsl:value-of select="ac7"/></td>
    <td align='right'><xsl:value-of select="ac8"/></td>
    <td align='right'><xsl:value-of select="ac9"/></td>
  </tr>
  </xsl:for-each>
</table>

</xsl:template>
</xsl:stylesheet>
<!-- end xsl -->

<!-- begin xsl -->
<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- format_id=E7-8A-C6-97-FA-15-9C-49-B8-C9-5D-A8-1D-60-6E-0A -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/critical_policy_status">

<table border="1">
  <tr bgcolor="#00AEEF" colspan="2">
    <th colspan="2">Signal Status</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>State</th>
    <th>Signal Count</th>
  </tr>
  <tr>
    <td>Sleep</td>
    <td align='right'><xsl:value-of select="critical_policy_statistics/sleep_signaled" /></td>
  </tr>
  <tr>
    <td>Hibernate</td>
    <td align='right'><xsl:value-of select="critical_policy_statistics/hibernate_signaled" /></td>
  </tr>
  <tr>
    <td>Shutdown</td>
    <td align='right'><xsl:value-of select="critical_policy_statistics/shutdown_signaled" /></td>
  </tr>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="6">
    <th colspan="6">Trip Point Status</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Index</th>
    <th>Name</th>
    <th>Temperature</th>
    <th>Warm</th>
    <th>Hot</th>
    <th>Critical</th>
  </tr>
  <xsl:for-each select="critical_trip_point_status/participant">
  <tr>
    <td align='right'><xsl:value-of select="index"/></td>
    <td><xsl:value-of select="name"/></td>
    <td align='right'><xsl:value-of select="temperature"/></td>
    <td align='right'><xsl:value-of select="specific_info/warm"/></td>
    <td align='right'><xsl:value-of select="specific_info/hot"/></td>
    <td align='right'><xsl:value-of select="specific_info/critical"/></td>
  </tr>
  </xsl:for-each>
</table>

</xsl:template>
</xsl:stylesheet>
<!-- end xsl -->

<!-- begin xsl -->
<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- format_id=06-5B-45-B9-49-79-C6-40-AB-F2-36-3A-70-C8-70-6C -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/lpm_policy_status">

<table border="1">
  <tr bgcolor="#00AEEF">
    <td>LPM Mode Boss</td>
    <td align='right'>
        <xsl:value-of select="lpm_mode_boss" />
    </td>
  </tr>
  <tr bgcolor="#00AEEF">
    <td>LPM Mode</td>
    <td align='right'>
      <xsl:value-of select="lpm_mode" />
    </td>
  </tr>
  <xsl:if test="lpm_mode='App Specific'">
  <tr bgcolor="#00AEEF">
    <td>Foreground App Name</td>
    <td align='right'>
      <xsl:value-of select="foreground_app_name" />
    </td>
  </tr>
  </xsl:if>
  <xsl:if test="lpm_mode_boss='OS Controlled' and lpm_mode='Standard'">
  <tr bgcolor="#00AEEF">
    <td>LPMSet Index</td>
    <td align='right'>
      <xsl:value-of select="lpmset_index" />
    </td>
  </tr>
  </xsl:if>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="10">
    <th colspan="10">LPM Current Entries</th>
  </tr>
  <tr colspan="10">
    <td bgcolor="#00AEEF">Version</td>
    <td align='right' colspan="7">
      <xsl:value-of select="lpm_table/version" />
    </td>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Target ACPI scope</th>
    <th>Target Index</th>
    <th>Domain Index</th>
    <th>Domain Type</th>
    <th>Control Knob</th>
    <th>Control Value</th>
    <th>Applied Control</th>
    <th>Applied Control Units</th>
  </tr>
  <xsl:for-each select="lpm_table/lpm_entries/lpm_entry">
    <tr>
      <td>
        <xsl:value-of select="target_acpi_scope"/>
      </td>
      <td>
        <xsl:value-of select="target_index"/>
      </td>
      <td>
          <xsl:value-of select="domain_index"/>
      </td>
      <td align='right'>
          <xsl:value-of select="domain_type"/>
      </td>
      <td align='right'>
        <xsl:value-of select="control_knob"/>
      </td>
      <td align='right'>
        <xsl:value-of select="control_value"/>
      </td>
      <td align='right'>
          <xsl:value-of select="applied_control"/>
      </td>
      <td align='right'>
          <xsl:value-of select="applied_control_units"/>
      </td>
    </tr>
  </xsl:for-each>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="10">
    <th colspan="10">LPM Standard Configuration Entries</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Target ACPI scope</th>
    <th>Target Index</th>
    <th>Domain Index</th>
    <th>Domain Type</th>
    <th>Control Knob</th>
    <th>Control Value</th>
  </tr>
  <xsl:for-each select="lpm_std_config/lpm_entry">
    <tr>
      <td>
        <xsl:value-of select="target_acpi_scope"/>
      </td>
      <td>
        <xsl:value-of select="target_index"/>
      </td>
      <td>
          <xsl:value-of select="domain_index"/>
      </td>
      <td align='right'>
          <xsl:value-of select="domain_type"/>
      </td>
      <td align='right'>
        <xsl:value-of select="control_knob"/>
      </td>
      <td align='right'>
        <xsl:value-of select="control_value"/>
      </td>
    </tr>
  </xsl:for-each>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="10">
    <th colspan="10">LPM Sets</th>
  </tr>
  <xsl:for-each select="lpm_sets/lpmset_entry">
    <tr colspan="10">
      <td bgcolor="#00AEEF">LpmSet Index</td>
      <td align='right' colspan="7">
        <xsl:value-of select="lpmset_index" />
      </td>
    </tr>
    <tr bgcolor="#00AEEF">
      <th>Target ACPI scope</th>
      <th>Target Index</th>
      <th>Domain Index</th>
      <th>Domain Type</th>
      <th>Control Knob</th>
      <th>Control Value</th>
    </tr>
    <xsl:for-each select="lpm_entries/lpm_entry">
      <tr>
        <td>
          <xsl:value-of select="target_acpi_scope"/>
        </td>
        <td>
          <xsl:value-of select="target_index"/>
        </td>
        <td>
            <xsl:value-of select="domain_index"/>
        </td>
        <td align='right'>
            <xsl:value-of select="domain_type"/>
        </td>
        <td align='right'>
          <xsl:value-of select="control_knob"/>
        </td>
        <td align='right'>
          <xsl:value-of select="control_value"/>
        </td>
      </tr>
    </xsl:for-each>
  </xsl:for-each>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="10">
    <th colspan="10">LPM App Specific Entries</th>
  </tr>
  <tr bgcolor="#00AEEF">
      <th>LpmSet Index</th>
      <th>App Names</th>
  </tr>
  <xsl:for-each select="lpm_app_entries/lpm_app_entry">
    <tr>
      <td align='right'>
        <xsl:value-of select="lpmset_index" />
      </td>
      <td align='right'>
        <xsl:value-of select="app_names" />
      </td>
    </tr>
  </xsl:for-each>
</table>

</xsl:template>
</xsl:stylesheet>
<!-- end xsl -->

<!-- begin xsl -->
<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- format_id=D6-41-A4-42-6A-AE-2B-46-A8-4B-4A-8C-E7-90-27-D3 -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/passive_policy_status">

<table border="1">
  <tr bgcolor="#00AEEF" colspan="7">
    <th colspan="7">Trip Point Status</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Participant</th>
    <th>Aux0</th>
    <th>Temp</th>
    <th>Aux1</th>
    <th>Hysteresis</th>
    <th>PSV</th>
    <th>NTT</th>
  </tr>
  <xsl:for-each select="passive_trip_point_status/participant">
  <tr>
    <td><xsl:value-of select="name"/> (<xsl:value-of select="index"/>)</td>
    <td align='right'><xsl:value-of select="temperature_thresholds/aux0"/></td>
    <td align='right'><xsl:value-of select="temperature"/></td>
    <td align='right'><xsl:value-of select="temperature_thresholds/aux1"/></td>
    <td align='right'><xsl:value-of select="temperature_thresholds/hysteresis"/></td>
    <td align='right'><xsl:value-of select="specific_info/passive"/></td>
    <td align='right'><xsl:value-of select="specific_info/ntt"/></td>
  </tr>
  </xsl:for-each>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="10">
    <th colspan="10">Control Status</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Participant</th>
    <th>Domain</th>
    <th>Temp</th>
    <th>Util</th>
    <th>Priority</th>
    <th>Power</th>
    <th>P-State</th>
    <th>Core</th>
    <th>T-State</th>
    <th>Display</th>
  </tr>
  <xsl:for-each select="passive_control_status/participant_control_status">
  <xsl:for-each select="domain_control_status">
  <tr>
    <td><xsl:value-of select="../name"/> (<xsl:value-of select="../index"/>)</td>
    <td><xsl:value-of select="name"/> (<xsl:value-of select="index"/>)</td>
    <td align='right'><xsl:value-of select="temperature"/></td>
    <td align='right'><xsl:value-of select="utilization"/></td>
    <td align='right'><xsl:value-of select="priority"/></td>
    <xsl:for-each select="controls/control">
    <xsl:choose>
    <xsl:when test="current='X' and max!='X' and min!='X'">
    <td align='center' bgcolor="#ED1C24"><xsl:value-of select="min"/>-<xsl:value-of select="current"/>-<xsl:value-of select="max"/></td>
    </xsl:when>
    <xsl:when test="current!=min">
    <td align='center' bgcolor="#FFFF00"><xsl:value-of select="min"/>-<xsl:value-of select="current"/>-<xsl:value-of select="max"/></td>
    </xsl:when>
    <xsl:otherwise>
    <td align='center'><xsl:value-of select="min"/>-<xsl:value-of select="current"/>-<xsl:value-of select="max"/></td>
    </xsl:otherwise>
    </xsl:choose>
    </xsl:for-each>
  </tr>
  </xsl:for-each>
  </xsl:for-each>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="4">
    <th colspan="4">TRT</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Target</th>
    <th>Source</th>
    <th>Influence</th>
    <th>Sample Period (s)</th>
  </tr>
  <xsl:for-each select="trt/trt_entry">
  <tr>
    <td><xsl:value-of select="target_acpi_scope"/> (<xsl:value-of select="target_index"/>)</td>
    <td><xsl:value-of select="source_acpi_scope"/> (<xsl:value-of select="source_index"/>)</td>
    <td align='right'><xsl:value-of select="influence"/></td>
    <td align='right'><xsl:value-of select="sampling_period"/></td>
  </tr>
  </xsl:for-each>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="3">
    <th colspan="3">Trip Point Statistics</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Participant</th>
    <th>Time Since Last Trip</th>
    <th>Temperature Of Last Trip</th>
  </tr>
  <xsl:for-each select="trip_point_statistics/participant_trip_point_statistics">
  <xsl:if test="supports_trip_points='true'">
  <tr>
    <td><xsl:value-of select="participant_name"/>(<xsl:value-of select="participant_index"/>)</td>
    <td align='right'><xsl:value-of select="time_since_last_trip"/></td>
    <td align='right'><xsl:value-of select="temperature_of_last_trip"/></td>
  </tr>
  </xsl:if>
  </xsl:for-each>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="2">
    <th colspan="2">Source Availability Schedule</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Source</th>
    <th>Next Time Available (S)</th>
  </tr>
  <xsl:for-each select="callback_scheduler/source_availability/activity">
  <tr>
    <td><xsl:value-of select="source"/></td>
    <td align='right'><xsl:value-of select="time_until_available"/></td>
  </tr>
  </xsl:for-each>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="2">
    <th colspan="2">Target Callback Schedule</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Target</th>
    <th>Next Callback Time (S)</th>
  </tr>
  <xsl:for-each select="callback_scheduler/policy_callback_scheduler/participant_callback">
  <tr>
    <td><xsl:value-of select="participant_index"/></td>
    <td align='right'><xsl:value-of select="time_until_expires"/></td>
  </tr>
  </xsl:for-each>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="1">
    <th colspan="1">Utilization Threshold</th>
  </tr>
  <tr>
   <td><xsl:value-of select="utilization_threshold"/></td>
  </tr>
 </table>
 
</xsl:template>
</xsl:stylesheet>
<!-- end xsl -->

<!-- begin xsl -->
<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- format_id=5A-11-04-9E-87-AE-1C-4D-95-00-0F-3E-34-0B-FE-75 -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/passive_policy2_status">
<!--  style="position: relative; top: 00px; left: 0px;"-->
    <div>
        <div>
            <table border="1">
              <tr bgcolor="#00AEEF" colspan="13">
                <th colspan="13">Targets</th>
              </tr>
              <tr bgcolor="#00AEEF">
                <th align='left' style='max-width:80px; min-width:80px;'>Target</th>
                <th align='left' style='max-width:90px; min-width:90px;'>Monitored</th>
                <th align='right' style='max-width:90px; min-width:90px;'>Resume Time</th>
                <th align='right' style='max-width:60px; min-width:60px;'>Aux0</th>
                <th align='right' style='max-width:60px; min-width:60px;'>Current Temp</th>
                <th align='right' style='max-width:60px; min-width:60px;'>Aux1</th>
                <th align='right' style='max-width:60px; min-width:60px;'>Hyst</th>
                <th align='left' style='max-width:80px; min-width:80px;'>Source</th>
                <th align='left' style='max-width:80px; min-width:80px;'>Domain</th>
                <th align='left' style='max-width:220px; min-width:220px;'>Control Knob</th>
                <th align='right' style='max-width:90px; min-width:90px;'>Requested Value</th>
                <th align='right' style='max-width:90px; min-width:90px;'>Granted Value</th>
                <th align='right' style='max-width:90px; min-width:90px;'>Preferred State</th>
              </tr>
              <xsl:for-each select="court_status/targets/target">
              <tr>
                <td><xsl:value-of select="target_name"/> (<xsl:value-of select="target_id"/>)</td>
                <td><xsl:value-of select="trial/is_in_session"/></td>
                <td align='right'><xsl:value-of select="trial/participant_callback/time_until_expires"/></td>
                <td align='right'><xsl:value-of select="temperature_thresholds/aux0"/></td>
                <td align='right'><xsl:value-of select="current_temperature"/></td>
                <td align='right'><xsl:value-of select="temperature_thresholds/aux1"/></td>
                <td align='right'><xsl:value-of select="temperature_thresholds/hysteresis"/></td>
                <td colspan='6'>
                  <table border="1">
                      <xsl:for-each select="trial/representative">
                        <tr>
                          <td style='max-width:80px; min-width:80px;'><xsl:value-of select="client_status/source_domain_knob/source_name"/> 
                            (<xsl:value-of select="client_status/source_domain_knob/source_index"/>)</td>
                          <td style='max-width:80px; min-width:80px;'><xsl:value-of select="client_status/source_domain_knob/domain_name"/> 
                            (<xsl:value-of select="client_status/source_domain_knob/domain_index"/>)</td>
                          <td style='max-width:220px; min-width:220px;'><xsl:value-of select="client_status/source_domain_knob/control_type"/></td>
                          <td align='right' style='max-width:90px; min-width:90px;'><xsl:value-of select="client_status/last_set_preferred_state"/></td>
                          <td align='right' style='max-width:90px; min-width:90px;'><xsl:value-of select="client_status/granted_value"/></td>
                          <td align='right' style='max-width:90px; min-width:90px;'><xsl:value-of select="targetted_preferred_state"/></td>
                        </tr>
                      </xsl:for-each>
                  </table>
                </td>
              </tr>
              </xsl:for-each>
            </table>
        </div>

        <br></br>
        
        <div style="width: 1188px">
            <table border="1" style="float: right;">
                <tr bgcolor="#00AEEF" colspan="6">
                    <th colspan="6">Sources</th>
                </tr>
                <tr bgcolor="#00AEEF">
                    <th align='left' style='max-width:80px; min-width:80px;'>Source</th>
                    <th align='left' style='max-width:80px; min-width:80px;'>Domain</th>
                    <th align='left' style='max-width:220px; min-width:220px;'>Control Type</th>
                    <th align='right' style='max-width:90px; min-width:90px;'>Max</th>
                    <th align='right' style='max-width:90px; min-width:90px;'>Value</th>
                    <th align='right' style='max-width:90px; min-width:90px;'>Min</th>
                </tr>
                <xsl:for-each select="court_status/clients/client">
                    <tr>
                        <td align='left'>
                            <xsl:value-of select="source_domain_knob/source_name"/> (<xsl:value-of select="source_domain_knob/source_index"/>)
                        </td>
                        <td align='left'>
                            <xsl:value-of select="source_domain_knob/domain_name"/> (<xsl:value-of select="source_domain_knob/domain_index"/>)
                        </td>
                        <td align='left'>
                            <xsl:value-of select="source_domain_knob/control_type"/>
                        </td>
                        <td align='right'>
                            <xsl:value-of select="max"/>
                        </td>
                        <xsl:choose>
                            <xsl:when test="val!=max">
                                <td align='right' bgcolor="#FFFF00">
                                    <xsl:value-of select="val"/>
                                </td>
                            </xsl:when>
                            <xsl:otherwise>
                                <td align='right'>
                                    <xsl:value-of select="val"/>
                                </td>
                            </xsl:otherwise>
                        </xsl:choose>
                        <td align='right'>
                            <xsl:value-of select="min"/>
                        </td>
                    </tr>
                </xsl:for-each>
            </table>

            <table border="1" style="float: left;">
                <tr bgcolor="#00AEEF" colspan="3">
                    <th colspan="3">Trip Point Statistics</th>
                </tr>
                <tr bgcolor="#00AEEF">
                    <th align='left' style='max-width:80px; min-width:80px;'>Target</th>
                    <th align='right' style='max-width:80px; min-width:80px;'>Time</th>
                    <th align='right' style='max-width:80px; min-width:80px;'>Temp</th>
                </tr>
                <xsl:for-each select="trip_point_statistics/participant_trip_point_statistics">
                    <xsl:if test="supports_trip_points='true'">
                        <tr>
                            <td>
                                <xsl:value-of select="participant_name"/> (<xsl:value-of select="participant_index"/>)
                            </td>
                            <td align='right'>
                                <xsl:value-of select="time_since_last_trip"/>
                            </td>
                            <td align='right'>
                                <xsl:value-of select="temperature_of_last_trip"/>
                            </td>
                        </tr>
                    </xsl:if>
                </xsl:for-each>
            </table>
        </div>

        <div style="clear: both;"></div>
        <br></br>
        
        <div>
            <table border="1">
                <tr bgcolor="#00AEEF">
                    <th colspan="11">
                        PSVT (<xsl:value-of select="psvt/control_mode"/>)
                    </th>
                </tr>
                <tr bgcolor="#00AEEF">
                    <th align='left'>Target</th>
                    <th align='left'>Source</th>
                    <th align='left'>Source Domain</th>
                    <th align='left'>Control Knob</th>
                    <th align='right' width='60px'>Priority</th>
                    <th align='right' width='60px'>Sample Period (s)</th>
                    <th align='right' width='60px'>Passive Temp</th>
                    <th align='right' width='60px'>Limit</th>
                    <th align='right' width='60px'>Step Size</th>
                    <th align='right' width='80px'>Limit Coefficient</th>
                    <th align='right' width='80px'>Unlimit Coefficient</th>
                </tr>
                <xsl:for-each select="psvt/psvt_entry">
                    <tr>
                        <td align='left'>
                            <xsl:value-of select="target_device_scope"/> (<xsl:value-of select="target_index"/>)
                        </td>
                        <td align='left'>
                            <xsl:value-of select="source_device_scope"/> (<xsl:value-of select="source_index"/>)
                        </td>
                        <td align='left'>
                            <xsl:value-of select="source_domain"/> (<xsl:value-of select="domain_index"/>)
                        </td>
                        <td align='left'>
                            <xsl:value-of select="control_knob"/>
                        </td>
                        <td align='right'>
                            <xsl:value-of select="passive_temperature_setting/priority"/>
                        </td>
                        <td align='right'>
                            <xsl:value-of select="passive_temperature_setting/sampling_period"/>
                        </td>
                        <td align='right'>
                            <xsl:value-of select="passive_temperature_setting/passive_temperature"/>
                        </td>
                        <td align='right'>
                            <xsl:value-of select="passive_temperature_setting/limit"/>
                        </td>
                        <td align='right'>
                            <xsl:value-of select="passive_temperature_setting/step_size"/>
                        </td>
                        <td align='right'>
                            <xsl:value-of select="passive_temperature_setting/limit_coefficient"/>
                        </td>
                        <td align='right'>
                            <xsl:value-of select="passive_temperature_setting/unlimit_coefficient"/>
                        </td>
                    </tr>
                </xsl:for-each>
            </table>
        </div>
    </div>

  </xsl:template>
</xsl:stylesheet>
<!-- end xsl -->

<!-- begin xsl -->
<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- format_id=0A-97-45-E1-C1-E4-73-4D-90-0E-C9-C5-A6-9D-D0-67 -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/config_tdp_policy_status">

<table border="1">
  <tr bgcolor="#00AEEF" colspan="5">
    <th colspan="5">Config TDP Levels</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Participant</th>
    <th>Domain</th>
    <th>Lower Config TDP</th>
    <th>Current Config TDP</th>
    <th>Upper Config TDP</th>
  </tr>
  <xsl:for-each select="config_tdp_levels/participant_config_tdp_level/domain_config_tdp_level">
  <xsl:if test="control_config_tdp_level/config_tdp_control_dynamic_caps">
  <tr>
    <td align='left'><xsl:value-of select="../participant_name"/> (<xsl:value-of select="../participant_index"/>)</td>
    <td align='left'><xsl:value-of select="domain_name"/> (<xsl:value-of select="domain_index"/>)</td>
    <td align='right'><xsl:value-of select="control_config_tdp_level/config_tdp_control_dynamic_caps/lower_limit_index"/></td>
    <td align='right'><xsl:value-of select="control_config_tdp_level/config_tdp_control_status/control_index"/></td>
    <td align='right'><xsl:value-of select="control_config_tdp_level/config_tdp_control_dynamic_caps/upper_limit_index"/></td>
  </tr>
  </xsl:if>
  </xsl:for-each>
</table>

<br></br>
    
<xsl:for-each select="config_tdp_levels/participant_config_tdp_level/domain_config_tdp_level">
<xsl:if test="control_config_tdp_level/config_tdp_control_set">
<table border="1">
  <tr bgcolor="#00AEEF" colspan="5">
    <th colspan="5">Config TDP Control Set [<xsl:value-of select="../participant_name"/> (<xsl:value-of select="../participant_index"/>), <xsl:value-of select="domain_name"/> (<xsl:value-of select="domain_index"/>)]
    </th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Index</th>
    <th>Turbo Ratio</th>
    <th>Max Power</th>
    <th>Max Frequency</th>
  </tr>
  <xsl:for-each select="control_config_tdp_level/config_tdp_control_set/config_tdp_control">
  <tr>
    <td align='left'><xsl:value-of select="position() - 1"/></td>
    <td align='right'><xsl:value-of select="tdp_ratio"/></td>
    <td align='right'><xsl:value-of select="tdp_power"/></td>
    <td align='right'><xsl:value-of select="tdp_frequency"/></td>
  </tr>
  </xsl:for-each>
</table>
<br></br>
</xsl:if>
</xsl:for-each>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="5">
    <th colspan="5">Config TDP Activity</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Name</th>
    <th>Value</th>
  </tr>
  <tr>
    <td align='left'>Last Config TDP Request Level</td>
    <td align='right'><xsl:value-of select="config_tdp_activity/last_config_tdp_request"/></td>
  </tr>
  <tr>
    <td align='left'>Time Since Last Config TDP Request</td>
    <td align='right'><xsl:value-of select="config_tdp_activity/time_since_last_config_tdp_request"/></td>
  </tr>
  <tr>
    <td align='left'>Number of Config TDP Requests</td>
    <td align='right'><xsl:value-of select="config_tdp_activity/number_of_config_tdp_requests"/></td>
  </tr>
  <tr>
    <td align='left'>Number of Power Capability Changes</td>
    <td align='right'><xsl:value-of select="config_tdp_activity/number_of_power_capability_changes"/></td>
  </tr>
  <tr>
    <td align='left'>Number of Performance Capability Changes</td>
    <td align='right'><xsl:value-of select="config_tdp_activity/number_of_performance_capability_changes"/></td>
  </tr>
  <tr>
    <td align='left'>Number of Config TDP Capability Changes</td>
    <td align='right'><xsl:value-of select="config_tdp_activity/number_of_config_tdp_capability_changes"/></td>
  </tr>
</table>
</xsl:template>
</xsl:stylesheet>
<!-- end xsl -->

<!-- begin xsl -->
<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- format_id=B7-F1-CA-16-38-DD-ED-40-B1-C1-1B-8A-19-13-D5-31 -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/cooling_mode_policy_status">

<table border="1">
  <tr bgcolor="#00AEEF" colspan="3">
    <th colspan="3">Cooling Mode Activity</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Property</th>
    <th>Current Value</th>
    <th># Times Changed</th>
  </tr>
  <xsl:for-each select="cooling_mode_activity/property">
  <tr>
    <td align='left'><xsl:value-of select="name"/></td>
    <td align='left'><xsl:value-of select="status"/></td>
    <td align='right'><xsl:value-of select="num_changes"/></td>
  </tr>
  </xsl:for-each>
</table>
    
<br></br>
    
<table border="1">
  <tr bgcolor="#00AEEF" colspan="2">
    <th colspan="2">Workload Hint Configuration</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Workload Hint Value</th>
    <th>Application Names</th>
  </tr>
  <xsl:for-each select="workload_hint_configuration/workload_group">
  <tr>
    <td align='left'><xsl:value-of select="id"/></td>
    <td align='right'>
        <xsl:for-each select="applications/application">
            <xsl:value-of select="current()"/><br/>
        </xsl:for-each>
    </td>
  </tr>
  </xsl:for-each>
</table>
    
<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="3">
    <th colspan="3">Participant SCP/DSCP Support</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Participant</th>
    <th>Supports DSCP</th>
    <th>Supports SCP</th>
  </tr>
  <xsl:for-each select="scp_dscp_support/participant">
  <tr>
    <td align='left'><xsl:value-of select="name"/> (<xsl:value-of select="index"/>)</td>
    <td align='left'><xsl:value-of select="supports_dscp"/></td>
    <td align='left'><xsl:value-of select="supports_scp"/></td>
  </tr>
  </xsl:for-each>
</table>
    
</xsl:template>
</xsl:stylesheet>
<!-- end xsl -->

<!-- begin xsl -->
<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- format_id=49-18-CE-C4-3A-24-F3-49-B8-D5-F9-70-02-F3-8E-6A -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/act_policy_status">

<table border="1">
  <tr bgcolor="#00AEEF" colspan="5">
    <th colspan="5">FIVR Devices</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Participant</th>
    <th>Domain</th>
    <th>Last Set Frequency</th>
  </tr>
  <xsl:for-each select="fivr_device_list/fivr_device">
  <tr>
    <td align='left'><xsl:value-of select="participant_name"/> (<xsl:value-of select="participant_index"/>)</td>
    <td align='left'><xsl:value-of select="domain_name"/> (<xsl:value-of select="domain_index"/>)</td>
    <td align='right'><xsl:value-of select="radio_frequency_control/last_set_frequency"/></td>
  </tr>
  </xsl:for-each>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="6">
    <th colspan="6">Radio Devices</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Participant</th>
    <th>Domain</th>
    <th>Center Frequency</th>
    <th>Left Spread</th>
    <th>Right Spread</th>
  </tr>
  <xsl:for-each select="radio_device_list/radio_device">
  <tr>
    <td align='left'><xsl:value-of select="participant_name"/> (<xsl:value-of select="participant_index"/>)</td>
    <td align='left'><xsl:value-of select="domain_name"/> (<xsl:value-of select="domain_index"/>)</td>
    <td align='right'><xsl:value-of select="radio_frequency_profile_data/center_frequency"/></td>
    <td align='right'><xsl:value-of select="radio_frequency_profile_data/left_frequency_spread"/></td>
    <td align='right'><xsl:value-of select="radio_frequency_profile_data/right_frequency_spread"/></td>
  </tr>
  </xsl:for-each>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="9">
    <th colspan="9">Pixel Clock Device Properties</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Participant</th>
    <th>Domain</th>
    <th>Deviation</th>
    <th>Upward Deviation</th>
    <th>Downward Deviation</th>
    <th>Channel Type</th>
    <th>SSC Enabled</th>
    <th>Spread Type</th>
    <th>Spread Percentage</th>
  </tr>

  <xsl:for-each select="pixel_clock_device_list/pixel_clock_device">
  <tr>
    <td align='left'><xsl:value-of select="participant_name"/> (<xsl:value-of select="participant_index"/>)</td>
    <td align='left'><xsl:value-of select="domain_name"/> (<xsl:value-of select="domain_index"/>)</td>
    <td align='right'><xsl:value-of select="pixel_clock_capabilities/deviation"/></td>
    <td align='right'><xsl:value-of select="pixel_clock_capabilities/upward_deviation"/></td>
    <td align='right'><xsl:value-of select="pixel_clock_capabilities/downward_deviation"/></td>
    <td align='right'><xsl:value-of select="pixel_clock_capabilities/channel_type"/></td>
    <td align='right'><xsl:value-of select="pixel_clock_capabilities/ssc_enabled"/></td>
    <td align='right'><xsl:value-of select="pixel_clock_capabilities/spread_type"/></td>
    <td align='right'><xsl:value-of select="pixel_clock_capabilities/spread_percentage"/></td>
  </tr>
  </xsl:for-each>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="4">
    <th colspan="4">Pixel Clock Device Frequencies</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Pixel Clock</th>
    <th>Panel Frequency</th>
    <th>SSC Enabled Nudge Frequency</th>
    <th>SSC Disabled Nudge Frequency</th>
  </tr>
<xsl:for-each select="pixel_clock_device_list/pixel_clock_device/pixel_clock_data_set/pixel_clock">
  <tr>
    <td align='right'><xsl:value-of select="pixel_clock_number"/></td>
    <td align='right'><xsl:value-of select="pixel_clock_data/panel_input_frequency"/></td>
    <td align='right'><xsl:value-of select="pixel_clock_data/ssc_enabled_nudge_frequency"/></td>
    <td align='right'><xsl:value-of select="pixel_clock_data/ssc_disabled_nudge_frequency"/></td>
  </tr>
  </xsl:for-each>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="4">
    <th colspan="4">Pixel Clock Candidate Sets</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Name</th>
    <th>Spread Type</th>
    <th>Spread %</th>
    <th>Candidate Sets</th>
  </tr>
  <xsl:for-each select="pixel_clock_device_list/pixel_clock_device/candidate_sets/candidate_set">
  <tr>
    <td align='left'><xsl:value-of select="name"/></td>
    <td align='left'><xsl:value-of select="pixel_clock_frequency_table/spread_type"/></td>
    <td align='right'><xsl:value-of select="pixel_clock_frequency_table/spread_percentage"/></td>
    <td align='right'>
      <table border="1">
        <tr bgcolor="#00AEEF">
          <th>Pixel Clock</th>
          <th>Frequency</th>
        </tr>
        <xsl:for-each select="pixel_clock_frequency_table/pixel_clock_frequencies/pixel_clock">
        <tr>
          <td align='right'><xsl:value-of select="pixel_clock_number"/></td>
          <td align='right'>
          <xsl:for-each select="frequency">
            <xsl:value-of select="current()"/><br/>
          </xsl:for-each>
          </td>
        </tr>
        </xsl:for-each>
      </table>
    </td>
  </tr>
  </xsl:for-each>
</table>
    
</xsl:template>
</xsl:stylesheet>
<!-- end xsl -->

<!-- begin xsl -->
<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- format_id=0F-27-BE-63-11-1C-FD-48-A6-F7-3A-F2-53-FF-3E-2D -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/adaptive_performance_policy_status">

<table border="1">
  <tr bgcolor="#00AEEF" colspan="40">
    <th colspan="40">Adaptive Performance Conditions Table (APCT)</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th bgcolor="#CCFFCC">Action</th>
    <th bgcolor="#FFB2FF">Cond0</th>
    <th bgcolor="#FFB2FF">Comp0</th>
    <th bgcolor="#FFB2FF">Arg0</th>
    <th bgcolor="#FFB2FF">Op0</th>
    <th bgcolor="#FFFFB2">Cond1</th>
    <th bgcolor="#FFFFB2">Comp1</th>
    <th bgcolor="#FFFFB2">Arg1</th>
    <th bgcolor="#FFFFB2">Op1</th>
    <th bgcolor="#FFB2FF">Cond2</th>
    <th bgcolor="#FFB2FF">Comp2</th>
    <th bgcolor="#FFB2FF">Arg2</th>
    <th bgcolor="#FFB2FF">Op2</th>
    <th bgcolor="#FFFFB2">Cond3</th>
    <th bgcolor="#FFFFB2">Comp3</th>
    <th bgcolor="#FFFFB2">Arg3</th>
    <th bgcolor="#FFFFB2">Op3</th>
    <th bgcolor="#FFB2FF">Cond4</th>
    <th bgcolor="#FFB2FF">Comp4</th>
    <th bgcolor="#FFB2FF">Arg4</th>
    <th bgcolor="#FFB2FF">Op4</th>
    <th bgcolor="#FFFFB2">Cond5</th>
    <th bgcolor="#FFFFB2">Comp5</th>
    <th bgcolor="#FFFFB2">Arg5</th>
    <th bgcolor="#FFFFB2">Op5</th>
    <th bgcolor="#FFB2FF">Cond6</th>
    <th bgcolor="#FFB2FF">Comp6</th>
    <th bgcolor="#FFB2FF">Arg6</th>
    <th bgcolor="#FFB2FF">Op6</th>
    <th bgcolor="#FFFFB2">Cond7</th>
    <th bgcolor="#FFFFB2">Comp7</th>
    <th bgcolor="#FFFFB2">Arg7</th>
    <th bgcolor="#FFFFB2">Op7</th>
    <th bgcolor="#FFB2FF">Cond8</th>
    <th bgcolor="#FFB2FF">Comp8</th>
    <th bgcolor="#FFB2FF">Arg8</th>
    <th bgcolor="#FFB2FF">Op8</th>
    <th bgcolor="#FFFFB2">Cond9</th>
    <th bgcolor="#FFFFB2">Comp9</th>
    <th bgcolor="#FFFFB2">Arg9</th>
  </tr>
  <xsl:for-each select="conditions_table/conditions_table_entry">
    <xsl:variable name="current_action">
      <xsl:value-of select="action_id"/>
    </xsl:variable>
    <tr>
      <td>
        <xsl:for-each select="../../actions_table/actions_table_entry[action_id=$current_action]">
      <xsl:choose>
            <xsl:when test="position()=1">
              <xsl:value-of select="action_set"/>
            </xsl:when>
      </xsl:choose>
        </xsl:for-each>
      </td>
      <xsl:choose>
        <xsl:when test="logical_operation">
          <xsl:for-each select="logical_operation">
            <xsl:choose>
                <xsl:when test="minterm/result!='false'">
                  <td align='center' bgcolor="#FFFF00"><xsl:value-of select="minterm/condition"/></td>
                  <td align='center' bgcolor="#FFFF00"><xsl:value-of select="minterm/comparison"/></td>
                  <td align='center' bgcolor="#FFFF00"><xsl:value-of select="minterm/argument"/></td>
                </xsl:when>
                <xsl:otherwise>
                  <td align='center'><xsl:value-of select="minterm/condition"/></td>
                  <td align='center'><xsl:value-of select="minterm/comparison"/></td>
                  <td align='center'><xsl:value-of select="minterm/argument"/></td>
                </xsl:otherwise>
              </xsl:choose>
            <td align='center'><xsl:value-of select="operator"/></td>
          </xsl:for-each>
        </xsl:when>
        <xsl:otherwise>
          <xsl:choose>
            <xsl:when test="/adaptive_performance_policy_status/active_action!=action_id">
              <td align='center' colspan='4'>No Op: Take Default Action</td>
            </xsl:when>
            <xsl:otherwise>
              <td align='center' colspan='4' bgcolor="#FFFF00">No Op: Take Default Action</td>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:otherwise>
      </xsl:choose>
    </tr>
  </xsl:for-each>
</table>

  <br></br>

  <table border="1">
    <tr bgcolor="#00AEEF" colspan="5">
      <th colspan="5">Adaptive Performance Actions Table (APAT)</th>
    </tr>
    <tr bgcolor="#00AEEF">
      <th bgcolor="#FFB2FF">Action Set</th>
      <th bgcolor="#FFB2FF">Participant Scope</th>
      <th bgcolor="#FFB2FF">Domain</th>
      <th bgcolor="#FFB2FF">Code</th>
      <th bgcolor="#FFB2FF">Argument</th>
    </tr>
    <xsl:for-each select="actions_table/actions_table_entry">
      <xsl:choose>
        <xsl:when test="/adaptive_performance_policy_status/active_action!=action_id">
          <tr>
            <td align='left'><xsl:value-of select="action_set"/></td>
            <td align='left'><xsl:value-of select="participant_scope"/> (<xsl:value-of select="participant_index"/>)</td>
            <td align='left'><xsl:value-of select="domain"/> (<xsl:value-of select="domain_index"/>)</td>
            <td align='left'><xsl:value-of select="code"/></td>
            <td align='left'><xsl:value-of select="argument"/></td>
          </tr>
        </xsl:when>
        <xsl:otherwise>
          <tr>
            <td align='left' bgcolor="#FFFF00"><xsl:value-of select="action_set"/></td>
            <td align='left' bgcolor="#FFFF00"><xsl:value-of select="participant_scope"/> (<xsl:value-of select="participant_index"/>)</td>
            <td align='left' bgcolor="#FFFF00"><xsl:value-of select="domain"/> (<xsl:value-of select="domain_index"/>)</td>
            <td align='left' bgcolor="#FFFF00"><xsl:value-of select="code"/></td>
            <td align='left' bgcolor="#FFFF00"><xsl:value-of select="argument"/></td>
          </tr>
        </xsl:otherwise>
      </xsl:choose>      
    </xsl:for-each>
  </table>

<br></br>

<table border="1" style="width:48%;float:left;">
  <tr bgcolor="#00AEEF" colspan="3">
    <th colspan="3">Conditions</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Type</th>
    <th>Is Supported</th>
    <th>Last Known Value</th>
  </tr>
  <xsl:for-each select="conditions_directory/condition">
    <tr>
      <td align='center'><xsl:value-of select="condition_type"/></td>
      <td align='center'><xsl:value-of select="is_valid"/></td>
      <td align='center'><xsl:value-of select="current_value"/></td>
    </tr>
  </xsl:for-each>
</table>

<table border="1" style="width:48%;float:right;">
  <tr bgcolor="#00AEEF" colspan="2">
    <th colspan="2">Workload Hint Configuration</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Workload Hint Value</th>
    <th>Application Names</th>
  </tr>
  <xsl:for-each select="workload_hint_configuration/workload_group">
    <tr>
      <td align='left'>
        <xsl:value-of select="id"/>
      </td>
      <td align='right'>
        <xsl:for-each select="applications/application">
          <xsl:value-of select="current()"/>
          <br/>
        </xsl:for-each>
      </td>
    </tr>
  </xsl:for-each>
</table>  

</xsl:template>
</xsl:stylesheet>
<!-- end xsl -->

<!-- begin xsl -->
<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- format_id=CD-8C-56-64-97-65-FC-4B-B9-D6-9D-33-85-40-13-CE -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/dbpt_policy_status">

<table border="1">
  <tr bgcolor="#00AEEF" colspan="1">
    <th colspan="1">State of Charge</th>
  </tr>
  <tr>
   <td><xsl:value-of select="state_of_charge"/></td>
  </tr>
 </table>
 
 <br></br>
 
<table border="1">
  <tr bgcolor="#00AEEF" colspan="6">
    <th colspan="6">PDRT Table</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>State Of Charge</th>
    <th>Target</th>
    <th>Domain</th>
    <th>Control Knob</th>
  </tr>
  <xsl:for-each select="pdrt/pdrt_entry">
  <tr>
    <td align='right'><xsl:value-of select="state_of_charge"/></td>
    <td><xsl:value-of select="target_acpi_scope"/> (<xsl:value-of select="target_index"/>)</td>
    <td><xsl:value-of select="domain_index"/> (<xsl:value-of select="domain_type"/>)</td>
    <td>
      <table border="1">
        <tr bgcolor="#00AEEF" colspan="2">
          <th>Type</th>
          <th>Value</th>        
        </tr>
        <xsl:for-each select="pdrt_control_knob">
          <tr>
            <td align='right'><xsl:value-of select="control_knob"/></td>
            <td align='right'><xsl:value-of select="control_value"/></td>          
          </tr>
        </xsl:for-each>           
      </table>
    </td>
  </tr>
  </xsl:for-each>
</table>

<br></br>

</xsl:template>
</xsl:stylesheet>
<!-- end xsl -->

<!-- begin xsl -->
<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- format_id=A7-22-D7-6E-40-92-A5-48-B4-79-31-EE-F7-23-D7-CF -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:template match="/virtual_sensor_policy_status">
    <table border="1">
      <xsl:for-each select="virtual_sensor_directory/virtual_sensor_control">
        <h2>
          <xsl:value-of select="participant_name" /> - <xsl:value-of select="participant_description" />
        </h2>
        <tr>
          <!-- Virtual Sensor details-->          
          <table border="1">
            <tr bgcolor="#00AEEF" colspan="2"><th colspan="2">Participant Properties</th></tr>
            <tr>
              <td>Participant Index</td>
              <td><xsl:value-of select="participant_index" /></td>
            </tr>
            <tr>
              <td>Domain Index</td>
              <td><xsl:value-of select="domain_index" /></td>
            </tr>
            <tr>
              <td>Device Scope</td>
              <td><xsl:value-of select="device_scope" /></td>
            </tr>
            <tr>
              <td>Last Calculated Temperature</td>
              <xsl:choose>
                <xsl:when test="last_calculated_temperature &gt;= 255">
                  <td style="color:red">
                      <xsl:value-of select="last_calculated_temperature" />
                  </td>
                </xsl:when>
                <xsl:otherwise>
                  <td>
                    <xsl:value-of select="last_calculated_temperature" />
                  </td>
                </xsl:otherwise>
              </xsl:choose>
            </tr>
            <tr>
              <td>Last Set Temperature (C)</td>
              <xsl:choose>
                <xsl:when test="last_set_temperature &gt;= 255">
                  <td style="color:red">
                    <xsl:value-of select="last_set_temperature" />
                  </td>
                </xsl:when>
                <xsl:otherwise>
                  <td>
                    <xsl:value-of select="last_set_temperature" />
                  </td>
                </xsl:otherwise>
              </xsl:choose>
            </tr>
            <tr>
              <td>Time to next poll (s)</td>
              <td>
                <xsl:value-of select="participant_callback/time_until_expires" />
              </td>
            </tr>
          </table>
          <br></br>
          <!-- End Virtual Sensor details-->
        </tr>
        <tr>
          <!-- Virtual Sensor Targets -->
          <table border="1">
            <tr bgcolor="#00AEEF" colspan="7">
              <th colspan="7">Targets</th>
            </tr>
            <tr bgcolor="#00AEEF">
              <th>Type</th>
              <th>Target</th>
              <th>Target Index</th>
              <th>Domain Type</th>
              <th>Domain Index</th>
              <th>Current Value</th>
              <th>Previous Value</th>
            </tr>
            <xsl:for-each select="targets/virtual_sensor_target">
              <tr>
                <td align="left"><xsl:value-of select="target_type"/></td>
                <td align="left"><xsl:value-of select="target_scope"/></td>
                <td align="right"><xsl:value-of select="target_index"/></td>
                <td align="right"><xsl:value-of select="domain_type"/></td>
                <td align="right"><xsl:value-of select="domain_index"/></td>
                <td align="right"><xsl:value-of select="current_value"/></td>
                <td align="right"><xsl:value-of select="last_read_value"/></td>
              </tr>
            </xsl:for-each>
          </table>
          <br></br>
          <!-- End Virtual Sensor Targets -->
        </tr>
        <tr>
          <!-- Virtual Sensor Tables -->
          <table>
            <tr>
              <td colspan="7" align="left" valign="top">
                <table border="1">
                  <tr bgcolor="#00AEEF" colspan="7">
                    <th colspan="7">Virtual Sensor Calibration Table</th>
                  </tr>
                  <tr bgcolor="#00AEEF">
                    <th>Target</th>
                    <th>Domain Type</th>
                    <th>Coefficient Type</th>
                    <th>Coefficient</th>
                    <th>Operation</th>
                    <th>Alpha</th>
                    <th>Trigger Point</th>
                  </tr>
                  <xsl:for-each select="vsct/vsct_entry">
                    <tr>
                      <td align="left">
                        <xsl:value-of select="target_device_scope"/> (<xsl:value-of select="target_index"/>)
                      </td>
                      <td align="left"><xsl:value-of select="target_domain"/></td>
                      <td align="left"><xsl:value-of select="coefficient_type"/></td>
                      <td align="right"><xsl:value-of select="calibration_setting/coefficient"/></td>
                      <td align="center"><xsl:value-of select="calibration_setting/operation_type"/></td>
                      <td align="right"><xsl:value-of select="calibration_setting/alpha"/></td>
                      <td align="right"><xsl:value-of select="trigger_point"/></td>
                    </tr>
                  </xsl:for-each>
                </table>
              </td>
              <td width="5"></td>
              <td colspan="2" align="right" valign="top">
                <table border="1">
                  <tr bgcolor="#00AEEF" colspan="2">
                    <th colspan="2">Virtual Sensor Polling Table</th>
                  </tr>
                  <tr bgcolor="#00AEEF">
                    <th>Virtual Temperature</th>
                    <th>Polling Period (ms)</th>
                  </tr>
                  <xsl:for-each select="vspt/vspt_entry">
                    <tr>
                      <td align="right"><xsl:value-of select="virtual_temperature"/></td>
                      <td align="right"><xsl:value-of select="polling_period"/></td>
                    </tr>
                  </xsl:for-each>
                </table>
              </td>
            </tr>
          </table>
          <br></br>
          <!-- End Virtual Sensor Tables -->
        </tr>
      </xsl:for-each>
    </table>

    <br></br>

  </xsl:template>
</xsl:stylesheet>
<!-- end xsl -->

<!-- begin xsl -->
<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- format_id=F0-CB-64-06-E4-2B-46-B5-9C-85-32-D1-A1-B7-CB-68 -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/participant">

<h2><xsl:value-of select="participant_properties/name" /> - <xsl:value-of select="participant_properties/description" /></h2>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="2">
    <th colspan="2">Participant Properties</th>
  </tr>
  <tr>
    <td>Bus Type</td>
    <td><xsl:value-of select="participant_properties/bus_type" /></td>
  </tr>
  <tr>
    <td>ACPI Device</td>
    <td><xsl:value-of select="participant_properties/acpi_device" /></td>
  </tr>
  <tr>
    <td>Object ID</td>
    <td><xsl:value-of select="participant_properties/acpi_scope" /></td>
  </tr>
</table>

<br></br>

<xsl:if test="specific_info">
<table border="1">
  <tr bgcolor="#00AEEF" colspan="15">
    <th colspan="15">Specific Info</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>CRT</th>
    <th>HOT</th>
    <th>WRM</th>
    <th>PSV</th>
    <th>NTT</th>
    <th>AC0</th>
    <th>AC1</th>
    <th>AC2</th>
    <th>AC3</th>
    <th>AC4</th>
    <th>AC5</th>
    <th>AC6</th>
    <th>AC7</th>
    <th>AC8</th>
    <th>AC9</th>
  </tr>
  <tr>
      <td align='right'><xsl:value-of select="specific_info/crt" /></td>
      <td align='right'><xsl:value-of select="specific_info/hot" /></td>
      <td align='right'><xsl:value-of select="specific_info/wrm" /></td>
      <td align='right'><xsl:value-of select="specific_info/psv" /></td>
      <td align='right'><xsl:value-of select="specific_info/ntt" /></td>
      <td align='right'><xsl:value-of select="specific_info/ac0" /></td>
      <td align='right'><xsl:value-of select="specific_info/ac1" /></td>
      <td align='right'><xsl:value-of select="specific_info/ac2" /></td>
      <td align='right'><xsl:value-of select="specific_info/ac3" /></td>
      <td align='right'><xsl:value-of select="specific_info/ac4" /></td>
      <td align='right'><xsl:value-of select="specific_info/ac5" /></td>
      <td align='right'><xsl:value-of select="specific_info/ac6" /></td>
      <td align='right'><xsl:value-of select="specific_info/ac7" /></td>
      <td align='right'><xsl:value-of select="specific_info/ac8" /></td>
      <td align='right'><xsl:value-of select="specific_info/ac9" /></td>
  </tr>
</table>
</xsl:if>

<xsl:for-each select="domains/domain">
  <h3>Domain #<xsl:value-of select="index" /> - <xsl:value-of select="name" /> (<xsl:value-of select="description" />)</h3>

  <!-- Domain Priority -->
  <xsl:if test="domain_controls/domain_priority/domain_priority">
    <table border="1">
    <tr bgcolor="#00AEEF" colspan="2">
      <th colspan="2">Domain Properties</th>
    </tr>
    <tr>
      <td>Priority</td>
      <td align='right'><xsl:value-of select="domain_controls/domain_priority/domain_priority" /></td>
    </tr>
    </table>
    <br></br>
  </xsl:if>
  <!-- END - Domain Priority -->

  <!-- Temperature Control -->
  <xsl:if test="domain_controls/temperature_control">
    <table border="1">
    <tr bgcolor="#00AEEF" colspan="4">
      <th colspan="4">Temperature Control</th>
    </tr>
    <tr bgcolor="#00AEEF">
      <th>Temperature</th>
      <xsl:if test="domain_controls/temperature_control/temperature_thresholds">
        <th>Aux 0</th>
        <th>Aux 1</th>
        <th>Hysteresis</th>
      </xsl:if>
    </tr>
    <tr>
      <td align='right'><xsl:value-of select="domain_controls/temperature_control/temperature_status" /></td>
      <xsl:if test="domain_controls/temperature_control/temperature_thresholds">
        <td align='right'><xsl:value-of select="domain_controls/temperature_control/temperature_thresholds/aux0" /></td>
        <td align='right'><xsl:value-of select="domain_controls/temperature_control/temperature_thresholds/aux1" /></td>
        <td align='right'><xsl:value-of select="domain_controls/temperature_control/temperature_thresholds/hysteresis" /></td>
      </xsl:if>
    </tr>
    </table>
    <br></br>
  </xsl:if>
  <!-- END - Temperature Control -->

  <!-- Performance Control -->
  <!-- Performance Control - Status -->
  <xsl:if test="domain_controls/performance_control">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="2">
        <th colspan="2">Performance Control Status</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Current Index</th>
      </tr>
      <tr>
        <td align='right'><xsl:value-of select="domain_controls/performance_control/performance_control_status/current_index" /></td>
      </tr>
    </table>
    <br></br>
  </xsl:if>
  <!-- END - Performance Control - Status -->

  <!-- Performance Control - Static Caps -->
  <xsl:if test="domain_controls/performance_control">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="2">
        <th colspan="2">Performance Control Static Caps</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Dynamic Performance Control States</th>
      </tr>
      <tr>
        <td><xsl:value-of select="domain_controls/performance_control/performance_control_static_caps/dynamic_performance_control_states" /></td>
      </tr>
    </table>
    <br></br>
  </xsl:if>
  <!-- END - Performance Control - Static Caps -->

  <!-- Performance Control - Dynamic Caps -->
  <xsl:if test="domain_controls/performance_control">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="2">
        <th colspan="2">Performance Control Dynamic Caps</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Upper Limit</th>
        <th>Lower Limit</th>
      </tr>
      <tr>
        <td align='right'><xsl:value-of select="domain_controls/performance_control/performance_control_dynamic_caps/upper_limit_index" /></td>
        <td align='right'><xsl:value-of select="domain_controls/performance_control/performance_control_dynamic_caps/lower_limit_index" /></td>
      </tr>
    </table>
    <br></br>
  </xsl:if>
  <!-- END : Performance Control - Dynamic Caps -->

  <!-- Performance Control Set -->
  <xsl:if test="domain_controls/performance_control">
    <!-- First check to see if the table contains any real frequency data -->
    <xsl:variable name="total_performance">
      <xsl:value-of select="sum(domain_controls/performance_control/performance_control_set/performance_control/transition_latency) + 
      sum(domain_controls/performance_control/performance_control_set/performance_control/control_absolute_value)"/>
    </xsl:variable>
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="7">
        <th colspan="7">Performance Control Set</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Index</th>
        <th>Type</th>
        <xsl:choose>
          <xsl:when test="$total_performance &gt; 0">
            <th>Control ID</th>
            <th>TDP Power (mW)</th>
            <th>Performance %</th>
            <th>Transition Latency (ms)</th>
            <th>Control Value</th>
          </xsl:when>
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </tr>
      <xsl:for-each select="domain_controls/performance_control/performance_control_set/performance_control">
      <tr>
        <td align='right'><xsl:value-of select="position() - 1" /></td>
        <td><xsl:value-of select="control_type" /></td>
        <xsl:choose>
          <xsl:when test="$total_performance &gt; 0">
            <td align='right'>
              <xsl:value-of select="control_id" />
            </td>
            <td align='right'>
              <xsl:value-of select="tdp_power" />
            </td>
            <td align='right'>
              <xsl:value-of select="performance_percentage" />
            </td>
            <td align='right'>
              <xsl:value-of select="transition_latency" />
            </td>
            <td align='right'>
              <xsl:value-of select="control_absolute_value" />&#160;<xsl:value-of select="value_units" />
            </td>
          </xsl:when>
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </tr>
      </xsl:for-each>
    </table>
    <br></br>
  </xsl:if>
  <!-- END : Performance Control Set -->
  <!-- END : Performance Control -->

  <!-- Power Status -->
  <xsl:if test="domain_controls/power_status">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="7">
        <th colspan="7">Power Status</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Power</th>
      </tr>
      <tr>
        <td align='right'><xsl:value-of select="domain_controls/power_status/power" /></td>
      </tr>
    </table>
    <br></br>
  </xsl:if>
  <!-- END : Power Status -->

  <!-- Power Control Status -->
  <xsl:if test="domain_controls/power_control/power_limit_set">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="3">
        <th colspan="5">Power Control Status</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Control Type</th>
        <th>Enabled</th>
        <th>Power Limit</th>
        <th>Time Window</th>
        <th>Duty Cycle</th>
      </tr>
      <xsl:for-each select="domain_controls/power_control/power_limit_set/power_limit">
      <tr>
        <td align='left'><xsl:value-of select="type" /></td>
        <td align='right'><xsl:value-of select="enabled" /></td>
        <td align='right'><xsl:value-of select="limit_value" /></td>
        <td align='right'><xsl:value-of select="time_window" /></td>
        <td align='right'><xsl:value-of select="duty_cycle" /></td>
      </tr>
      </xsl:for-each>
    </table>
    <br></br>
  </xsl:if>
  <!-- END : Power Control Status -->

  <!-- Power Control Dynamic Caps -->
  <xsl:if test="domain_controls/power_control/power_control_dynamic_caps_set">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="6">
        <th colspan="8">Power Control Dynamic Caps</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Control Type</th>
        <th>Max Power Limit (mW)</th>
        <th>Min Power Limit (mW)</th>
        <th>Power Step Size</th>
        <th>Max Time Window</th>
        <th>Min Time Window</th>
      </tr>
      <xsl:for-each select="domain_controls/power_control/power_control_dynamic_caps_set/power_control_dynamic_caps">
      <tr>
        <td><xsl:value-of select="control_type" /></td>
        <td align='right'><xsl:value-of select="max_power_limit" /></td>
        <td align='right'><xsl:value-of select="min_power_limit" /></td>
        <td align='right'><xsl:value-of select="power_step_size" /></td>
        <td align='right'><xsl:value-of select="max_time_window" /></td>
        <td align='right'><xsl:value-of select="min_time_window" /></td>
      </tr>
      </xsl:for-each>
    </table>
    <br></br>
  </xsl:if>
  <!-- END : Power Control Dynamic Caps -->

  <!-- Core Control -->
  <xsl:if test="domain_controls/core_control">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="2">
        <th colspan="2">Core Control</th>
      </tr>
      <tr>
        <td>Active Logical Processors</td>
        <td align='right'><xsl:value-of select="domain_controls/core_control/core_control_status/active_logical_processors" /></td>
      </tr>
      <tr>
        <td>Total Logical Processors</td>
        <td align='right'><xsl:value-of select="domain_controls/core_control/core_control_static_caps/total_logical_processors" /></td>
      </tr>
      <tr>
        <td>Max Active Cores</td>
        <td align='right'><xsl:value-of select="domain_controls/core_control/core_control_dynamic_caps/max_active_cores" /></td>
      </tr>
      <tr>
        <td>Min Active Cores</td>
        <td align='right'><xsl:value-of select="domain_controls/core_control/core_control_dynamic_caps/min_active_cores" /></td>
      </tr>
      <tr>
        <td>LPO Enabled</td>
        <td><xsl:value-of select="domain_controls/core_control/core_control_lpo_preference/lpo_enabled" /></td>
      </tr>
      <tr>
        <td>Start P-State</td>
        <td align='right'><xsl:value-of select="domain_controls/core_control/core_control_lpo_preference/start_p_state" /></td>
      </tr>
      <tr>
        <td>Power Offlining Mode</td>
        <td><xsl:value-of select="domain_controls/core_control/core_control_lpo_preference/power_control_offlining_mode" /></td>
      </tr>
      <tr>
        <td>Performance Offlining Mode</td>
        <td><xsl:value-of select="domain_controls/core_control/core_control_lpo_preference/performance_control_offlining_mode" /></td>
      </tr>
    </table>
  <br></br>
  </xsl:if>
  <!-- END : Core Control -->

  <!-- Display Control -->
  <xsl:if test="domain_controls/display_control">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="1">
        <th colspan="7">Display Status</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Brightness</th>
      </tr>
      <tr>
        <td align='right'><xsl:value-of select="domain_controls/display_control/display_control_status/brightness_limit_index" /></td>
      </tr>
    </table>

    <br></br>

    <table border="1">
      <tr bgcolor="#00AEEF" colspan="2">
        <th colspan="2">Display Control Dynamic Caps</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Upper Limit</th>
        <th>Lower Limit</th>
      </tr>
      <tr>
        <td align='right'><xsl:value-of select="domain_controls/display_control/display_control_dynamic_caps/upper_limit_index" /></td>
        <td align='right'><xsl:value-of select="domain_controls/display_control/display_control_dynamic_caps/lower_limit_index" /></td>
      </tr>
    </table>

    <br></br>

    <table border="1">
      <tr bgcolor="#00AEEF" colspan="2">
        <th colspan="2">Display Control Set</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Index</th>
        <th>Brightness %</th>
      </tr>
      <xsl:for-each select="domain_controls/display_control/display_control_set/display_control">
      <tr>
        <td align='right'><xsl:value-of select="position() - 1" /></td>
        <td align='right'><xsl:value-of select="brightness" />%</td>
      </tr>
      </xsl:for-each>
    </table>

    <br></br>

  </xsl:if>
  <!-- END: Display Control -->

  <!-- Active Control -->
  <xsl:if test="domain_controls/active_control">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="2">
        <th colspan="2">Active Control Status</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Control ID / Fan Speed %</th>
        <th>Speed (RPM) for Control ID</th>
      </tr>
      <tr>
        <td align='right'><xsl:value-of select="domain_controls/active_control/active_control_status/current_control_id" /></td>
        <td align='right'><xsl:value-of select="domain_controls/active_control/active_control_status/current_speed" /></td>
      </tr>
    </table>

    <br></br>

    <table border="1">
      <tr bgcolor="#00AEEF" colspan="3">
        <th colspan="3">Active Control Static Caps</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Fine Grained Control</th>
        <th>Low Speed Notification</th>
        <th>Step Size</th>
      </tr>
      <tr>
        <td><xsl:value-of select="domain_controls/active_control/active_control_static_caps/fine_grained_control" /></td>
        <td><xsl:value-of select="domain_controls/active_control/active_control_static_caps/low_speed_notification" /></td>
        <td align='right'><xsl:value-of select="domain_controls/active_control/active_control_static_caps/step_size" /></td>
      </tr>
    </table>

    <br></br>

    <table border="1">
      <tr bgcolor="#00AEEF" colspan="5">
        <th colspan="5">Active Control Set</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Control ID</th>
        <th>Trip Point</th>
        <th>Speed (RPMs)</th>
        <th>Noise Level</th>
        <th>Power (mW)</th>
      </tr>
      <xsl:for-each select="domain_controls/active_control/active_control_set/active_control">
      <tr>
        <td align='right'><xsl:value-of select="control_id" /></td>
        <td align='right'><xsl:value-of select="trip_point" /></td>
        <td align='right'><xsl:value-of select="speed" /></td>
        <td align='right'><xsl:value-of select="noise_level" /></td>
        <td align='right'><xsl:value-of select="power" /></td>
      </tr>
      </xsl:for-each>
    </table>

    <br></br>

  </xsl:if>
  <!-- END: Active Control -->
    
    
  <!-- Hardware Duty Cycle Control -->
  <xsl:if test="domain_controls/hardware_duty_cycle_control">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="4">
        <th colspan="2">Hardware Duty Cycle Control Status</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Name</th>
        <th>Value</th>
      </tr>
      <tr>
        <td align='center'>Control Version</td>
        <td align='center'><xsl:value-of select="domain_controls/hardware_duty_cycle_control/control_knob_version" /></td>
      </tr>
      <tr>
        <td align='center'>Supported By Platform</td>
        <td align='center'><xsl:value-of select="domain_controls/hardware_duty_cycle_control/is_supported_by_platform" /></td>
      </tr>
      <tr>
        <td align='center'>Supported By Operating System</td>
        <td align='center'><xsl:value-of select="domain_controls/hardware_duty_cycle_control/is_supported_by_operating_system" /></td>
      </tr>
      <tr>
        <td align='center'>Current Duty Cycle (%)</td>
        <td align='center'><xsl:value-of select="domain_controls/hardware_duty_cycle_control/hardware_duty_cycle" /></td>
      </tr>
    </table>

    <br></br>
      
  </xsl:if>
  <!-- END: Hardware Duty Cycle Control -->
    
    
  <!-- Platform Power Control -->
  <xsl:if test="domain_controls/platform_power_control">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="5">
        <th colspan="5">Platform Power Control Status (<xsl:value-of select="domain_controls/platform_power_control/control_knob_version" />)</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Control</th>
        <th>Enabled</th>
        <th>Power Limit</th>
        <th>Time Window</th>
        <th>Duty Cycle</th>
      </tr>
      <xsl:for-each select="domain_controls/platform_power_control/platform_power_limit_set/platform_power_limit">
      <tr>
        <td align='right'><xsl:value-of select="type" /></td>
        <td align='right'><xsl:value-of select="enabled" /></td>
        <td align='right'><xsl:value-of select="limit_value" /></td>
        <td align='right'><xsl:value-of select="time_window" /></td>
        <td align='right'><xsl:value-of select="duty_cycle" /></td>
      </tr>
      </xsl:for-each>
    </table>
    <br></br>
  </xsl:if>
  <!-- END: Platform Power Control -->

  <!-- Platform Power Status -->
  <xsl:if test="domain_controls/platform_power_status">

    <xsl:for-each select="domain_controls/platform_power_status">      
      <table border="1">
        <tr bgcolor="#00AEEF" colspan="2">
          <th colspan="2">
            Platform Power Status (<xsl:value-of select="control_knob_version"/>)
          </th>
        </tr>
        <tr>
          <th>Max Battery Power (PMAX)</th>
          <td align='right'>
            <xsl:value-of select="max_battery_power" />
          </td>
        </tr>
        <tr>
          <th>Adapter Power (APWR)</th>
          <td align='right'>
            <xsl:value-of select="adapter_power" />
          </td>
        </tr>
        <tr>
          <th>Platform Power Consumption (NPWR)</th>
          <td align='right'>
            <xsl:value-of select="platform_power_consumption" />
          </td>
        </tr>
        <tr>
          <th>Platform Rest Of Power (PROP)</th>
          <td align='right'>
            <xsl:value-of select="platform_rest_of_power" />
          </td>
        </tr>
        <tr>
          <th>Adapter Power Rating (ARTG)</th>
          <td align='right'>
            <xsl:value-of select="adapter_power_rating" />
          </td>
        </tr>
        <tr>
          <th>Platform Power Source (PSRC)</th>
          <td align='right'>
            <xsl:value-of select="platform_power_source" />
          </td>
        </tr>
        <tr>
          <th>Charger Type (CTYP)</th>
          <td align='right'>
            <xsl:value-of select="charger_type" />
          </td>
        </tr>
        <tr>
          <th>Platform State Of Charge (PSOC)</th>
          <td align='right'>
            <xsl:value-of select="platform_state_of_charge" />
          </td>
        </tr>
        <tr>
          <th>AC Peak Power (APKP)</th>
          <td align='right'>
            <xsl:value-of select="ac_peak_power" />
          </td>
        </tr>
        <tr>
          <th>AC Peak Time Window (APKT)</th>
          <td align='right'>
            <xsl:value-of select="ac_peak_time_window" />
          </td>
        </tr>
    </table>
    </xsl:for-each>
    <br></br>
  </xsl:if>
  <!-- END: Platform Power Status -->

</xsl:for-each>
</xsl:template>
</xsl:stylesheet>
<!-- end xsl -->

<!-- begin xsl -->
<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- format_id=C5-61-4D-E9-30-80-4D-B5-98-1A-D1-D1-67-DD-4C-D7 -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/work_item_queue_manager_status">

<table border="1">
  <tr bgcolor="#00AEEF" colspan="4">
    <th colspan="4">Queue Statistics</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Queue</th>
    <th>Current Count</th>
    <th>Max Count</th>
    <th>Total Executed</th>
  </tr>
  <tr>
    <td>Immediate</td>
    <td align='right'>
      <xsl:value-of select="immediate_queue_statistics/current_count" />
    </td>
    <td align='right'>
      <xsl:value-of select="immediate_queue_statistics/max_count" />
    </td>
    <td align='right'>
      <xsl:value-of select="work_item_statistics/total_immediate_work_items_executed" />
    </td>
  </tr>
  <tr>
    <td>Deferred</td>
    <td align='right'>
      <xsl:value-of select="deferred_queue_statistics/current_count" />
    </td>
    <td align='right'>
      <xsl:value-of select="deferred_queue_statistics/max_count" />
    </td>
    <td align='right'>
      <xsl:value-of select="work_item_statistics/total_deferred_work_items_executed" />
    </td>
  </tr>
</table>

<br></br>

<table border="1">
  <tr bgcolor="#00AEEF" colspan="8">
    <th colspan="8">Work Item Statistics</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Work Item Type</th>
    <th>Total Executed</th>
    <th>Avg Execution Time</th>
    <th>Min Execution Time</th>
    <th>Max Execution Time</th>
    <th>Avg Queue Time</th>
    <th>Min Queue Time</th>
    <th>Max Queue Time</th>
  </tr>
  <xsl:for-each select="work_item_statistics/immediate_work_item_statistics/work_item">
    <tr>
      <td>
        <xsl:value-of select="work_item_type" />
      </td>
      <td align='right'>
        <xsl:value-of select="total_executed" />
      </td>
      <td align='right'>
        <xsl:value-of select="average_execution_time" />
      </td>
      <td align='right'>
        <xsl:value-of select="min_execution_time" />
      </td>
      <td align='right'>
        <xsl:value-of select="max_execution_time" />
      </td>
      <td align='right'>
        <xsl:value-of select="average_queue_time" />
      </td>
      <td align='right'>
        <xsl:value-of select="min_queue_time" />
      </td>
      <td align='right'>
        <xsl:value-of select="max_queue_time" />
      </td>
    </tr>
  </xsl:for-each>
</table>

</xsl:template>
</xsl:stylesheet>
<!-- end xsl -->

<!-- begin xsl -->
<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- format_id=BF-BA-84-BE-D4-C4-3D-40-B4-95-31-28-FD-44-DA-C1 -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/hdc_policy_status">

    <table border="1">
        <tr bgcolor="#00AEEF" colspan="1">
            <th colspan="1">Operating System HDC Status</th>
        </tr>

        <xsl:choose>
            <xsl:when test="os_hdc_status/status='Enabled'">
                <tr bgcolor="lime">
                    <th colspan="1">
                        <xsl:value-of select="os_hdc_status/status"/>
                    </th>
                </tr>
            </xsl:when>
            <xsl:when test="os_hdc_status/status='Disabled'">
                <tr bgcolor="lightgray">
                    <th colspan="1">
                        <xsl:value-of select="os_hdc_status/status"/>
                    </th>
                </tr>
            </xsl:when>
            <xsl:otherwise>
                <tr bgcolor="white">
                    <th colspan="1">
                        <xsl:value-of select="os_hdc_status/status"/>
                    </th>
                </tr>
            </xsl:otherwise>
        </xsl:choose>
    </table>
    
    <br></br>
    
    <table border="1">
        <tr bgcolor="#00AEEF" colspan="9">
            <th colspan="10">HDC Control List</th>
        </tr>
        <tr bgcolor="#00AEEF">
            <th>Participant</th>
            <th>Domain</th>
            <th>Cycle Num</th>
            <th>Last Utilization Saturated</th>
            <th>Last Set Duty Cycle (%)</th>
            <th>HDC Supported</th>
            <th>HWP Supported</th>
            <th>Graphics % Active</th>
            <th>CPU Package % Active</th>
            <th>CPU LP % Active</th>
        </tr>
        <xsl:for-each select="hdc_control_list/hdc_control">
            <tr>
                <td>
                    <xsl:value-of select="domain/participant_properties/name" /> (<xsl:value-of select="domain/participant_index" />)
                </td>
                <td>
                    <xsl:value-of select="domain/domain_properties/name" /> (<xsl:value-of select="domain/domain_index" />)
                </td>
                <td>
                    <xsl:value-of select="cycle_num" />
                </td>
                <td>
                    <xsl:value-of select="last_util_saturated" />
                </td>
                <td>
                    <xsl:value-of select="last_set_duty_cycle" />
                </td>
                <td>
                    <xsl:value-of select="supported_by_platform" />
                </td>
                <td>
                    <xsl:value-of select="supported_by_operating_system" />
                </td>
                <td>
                    <xsl:value-of select="hardware_duty_cycle_utilization_set/graphics_utilization" />
                </td>
                <td>
                    <xsl:value-of select="hardware_duty_cycle_utilization_set/processor_package_utilization" />
                </td>
                <td>
                    <xsl:for-each select="hardware_duty_cycle_utilization_set/processor_core_utilization/core_utilization">
                        LP[<xsl:value-of select="id" />] = <xsl:value-of select="utilization" /><br></br>
                    </xsl:for-each>
                </td>
            </tr>
        </xsl:for-each>
    </table>

    <br></br>

    <table border="1">
        <tr bgcolor="#00AEEF" colspan="2">
            <th colspan="2">Periodic Timer</th>
        </tr>
        <tr bgcolor="#00AEEF">
            <th>Name</th>
            <th>Value</th>
        </tr>
        <tr>
            <td>Timer Started</td>
            <td>
                <xsl:value-of select="periodic_timer/timer_started" />
            </td>
        </tr>
        <tr>
            <td>Timer Period (ms)</td>
            <td>
                <xsl:value-of select="periodic_timer/timer_period" />
            </td>
        </tr>
        <tr>
            <td>Registered Objects</td>
            <td>
                <xsl:for-each select="periodic_timer/registered_objects/registered_object">
                    <xsl:value-of select="hdc_control/domain/participant_properties/name" /> (<xsl:value-of select="hdc_control/domain/participant_index" />) - <xsl:value-of select="hdc_control/domain/domain_properties/name" /> (<xsl:value-of select="hdc_control/domain/domain_index" />)<br></br>
                </xsl:for-each>
            </td>
        </tr>
    </table>
  
  <br></br>

  <table border="1">
      <tr bgcolor="#00AEEF" colspan="2">
          <th colspan="2">HDC Control Configurable Parameters</th>
      </tr>
      <tr bgcolor="#00AEEF">
          <th>Name</th>
          <th>Value</th>
      </tr>
      <tr>
          <td>SemiActive Workload Utilization Threshold (%)</td>
          <td>
              <xsl:value-of select="hdc_configuration_params/semi_active_workload_utilization_threshold" />
          </td>
      </tr>
      <tr>
          <td>Unaligned Workload Detection Multiplier</td>
          <td>
              <xsl:value-of select="hdc_configuration_params/unaligned_workload_detection_multiplier" />
          </td>
      </tr>
      <tr>
          <td>Unaligned Workload Detection Threshold</td>
          <td>
              <xsl:value-of select="hdc_configuration_params/unaligned_workload_detection_threshold" />
          </td>
      </tr>
      <tr>
          <td>DutyCycle GuardBand (%)</td>
          <td>
              <xsl:value-of select="hdc_configuration_params/duty_cycle_guard_band" />
          </td>
      </tr>
      <tr>
          <td>Max num cycles</td>
          <td>
              <xsl:value-of select="hdc_configuration_params/max_num_cycles" />
          </td>
      </tr>
      <tr>
          <td>Max duty cycle %</td>
          <td>
              <xsl:value-of select="hdc_configuration_params/max_duty_cycle" />
          </td>
      </tr>
      <tr>
          <td>HWP override</td>
          <td>
              <xsl:value-of select="hdc_configuration_params/hwp_override" />
          </td>
      </tr>
      <tr>
          <td>HDC OOB override</td>
          <td>
              <xsl:value-of select="hdc_configuration_params/hdc_oob_override" />
          </td>
      </tr>
  </table>

</xsl:template>
</xsl:stylesheet>
<!-- end xsl -->


<!-- begin xsl -->
<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- format_id=14-50-A3-F5-09-C2-A4-46-99-3A-EB-56-DE-75-30-A1 -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:template match="/power_boss_policy_status">
      <table border="1">
        <tr bgcolor="#00AEEF" colspan="40">
          <th colspan="40">Power Boss Conditions Table (PBCT)</th>
        </tr>
        <tr bgcolor="#00AEEF">
          <th bgcolor="#CCFFCC">Action</th>
          <th bgcolor="#FFB2FF">Cond0</th>
          <th bgcolor="#FFB2FF">Comp0</th>
          <th bgcolor="#FFB2FF">Arg0</th>
          <th bgcolor="#FFB2FF">Op0</th>
          <th bgcolor="#FFFFB2">Cond1</th>
          <th bgcolor="#FFFFB2">Comp1</th>
          <th bgcolor="#FFFFB2">Arg1</th>
          <th bgcolor="#FFFFB2">Op1</th>
          <th bgcolor="#FFB2FF">Cond2</th>
          <th bgcolor="#FFB2FF">Comp2</th>
          <th bgcolor="#FFB2FF">Arg2</th>
          <th bgcolor="#FFB2FF">Op2</th>
          <th bgcolor="#FFFFB2">Cond3</th>
          <th bgcolor="#FFFFB2">Comp3</th>
          <th bgcolor="#FFFFB2">Arg3</th>
          <th bgcolor="#FFFFB2">Op3</th>
          <th bgcolor="#FFB2FF">Cond4</th>
          <th bgcolor="#FFB2FF">Comp4</th>
          <th bgcolor="#FFB2FF">Arg4</th>
          <th bgcolor="#FFB2FF">Op4</th>
          <th bgcolor="#FFFFB2">Cond5</th>
          <th bgcolor="#FFFFB2">Comp5</th>
          <th bgcolor="#FFFFB2">Arg5</th>
          <th bgcolor="#FFFFB2">Op5</th>
          <th bgcolor="#FFB2FF">Cond6</th>
          <th bgcolor="#FFB2FF">Comp6</th>
          <th bgcolor="#FFB2FF">Arg6</th>
          <th bgcolor="#FFB2FF">Op6</th>
          <th bgcolor="#FFFFB2">Cond7</th>
          <th bgcolor="#FFFFB2">Comp7</th>
          <th bgcolor="#FFFFB2">Arg7</th>
          <th bgcolor="#FFFFB2">Op7</th>
          <th bgcolor="#FFB2FF">Cond8</th>
          <th bgcolor="#FFB2FF">Comp8</th>
          <th bgcolor="#FFB2FF">Arg8</th>
          <th bgcolor="#FFB2FF">Op8</th>
          <th bgcolor="#FFFFB2">Cond9</th>
          <th bgcolor="#FFFFB2">Comp9</th>
          <th bgcolor="#FFFFB2">Arg9</th>
        </tr>
        <xsl:for-each select="conditions_table/conditions_table_entry">
          <xsl:variable name="current_action">
            <xsl:value-of select="action_id"/>
          </xsl:variable>
          <tr>
            <td>
              <xsl:for-each select="../../actions_table/actions_table_entry[action_id=$current_action]">
                <xsl:choose>
                  <xsl:when test="position()=1">
                    <xsl:value-of select="action_set"/>
                  </xsl:when>
                </xsl:choose>
              </xsl:for-each>
            </td>
            <xsl:choose>
              <xsl:when test="logical_operation">
                <xsl:for-each select="logical_operation">
                  <xsl:choose>
                    <xsl:when test="minterm/result!='false'">
                      <td align='center' bgcolor="#FFFF00">
                        <xsl:value-of select="minterm/condition"/>
                      </td>
                      <td align='center' bgcolor="#FFFF00">
                        <xsl:value-of select="minterm/comparison"/>
                      </td>
                      <td align='center' bgcolor="#FFFF00">
                        <xsl:value-of select="minterm/argument"/>
                      </td>
                    </xsl:when>
                    <xsl:otherwise>
                      <td align='center'>
                        <xsl:value-of select="minterm/condition"/>
                      </td>
                      <td align='center'>
                        <xsl:value-of select="minterm/comparison"/>
                      </td>
                      <td align='center'>
                        <xsl:value-of select="minterm/argument"/>
                      </td>
                    </xsl:otherwise>
                  </xsl:choose>
                  <td align='center'>
                    <xsl:value-of select="operator"/>
                  </td>
                </xsl:for-each>
              </xsl:when>
              <xsl:otherwise>
                <xsl:choose>
                  <xsl:when test="/power_boss_policy_status/active_action!=action_id">
                    <td align='center' colspan='4'>No Op: Take Default Action</td>
                  </xsl:when>
                  <xsl:otherwise>
                    <td align='center' colspan='4' bgcolor="#FFFF00">No Op: Take Default Action</td>
                  </xsl:otherwise>
                </xsl:choose>
              </xsl:otherwise>
            </xsl:choose>
          </tr>
        </xsl:for-each>
      </table>
      
      <br></br>
      
      <table border="1">
        <tr bgcolor="#00AEEF" colspan="5">
          <th colspan="5">Power Boss Actions Table (PBAT)</th>
        </tr>
        <tr bgcolor="#00AEEF">
          <th bgcolor="#FFB2FF">Action Set</th>
          <th bgcolor="#FFB2FF">Participant Scope</th>
          <th bgcolor="#FFB2FF">Domain</th>
          <th bgcolor="#FFB2FF">Code</th>
          <th bgcolor="#FFB2FF">Argument</th>
        </tr>
        <xsl:for-each select="actions_table/actions_table_entry">
          <xsl:choose>
            <xsl:when test="/power_boss_policy_status/active_action!=action_id">
              <tr>
                <td align='left'><xsl:value-of select="action_set"/></td>
                <td align='left'><xsl:value-of select="participant_scope"/> (<xsl:value-of select="participant_index"/>)</td>
                <td align='left'><xsl:value-of select="domain"/> (<xsl:value-of select="domain_index"/>)</td>
                <td align='left'><xsl:value-of select="code"/></td>
                <td align='left'><xsl:value-of select="argument"/></td>
              </tr>
            </xsl:when>
            <xsl:otherwise>
              <tr>
                <td align='left' bgcolor="#FFFF00"><xsl:value-of select="action_set"/></td>
                <td align='left' bgcolor="#FFFF00"><xsl:value-of select="participant_scope"/> (<xsl:value-of select="participant_index"/>)</td>
                <td align='left' bgcolor="#FFFF00"><xsl:value-of select="domain"/> (<xsl:value-of select="domain_index"/>)</td>
                <td align='left' bgcolor="#FFFF00"><xsl:value-of select="code"/></td>
                <td align='left' bgcolor="#FFFF00"><xsl:value-of select="argument"/></td>
              </tr>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:for-each>
      </table>
      
      <br></br>

      <table border="1" style="width:48%;float:left;">
        <tr bgcolor="#00AEEF" colspan="3">
          <th colspan="3">Conditions</th>
        </tr>
        <tr bgcolor="#00AEEF">
          <th>Type</th>
          <th>Is Supported</th>
          <th>Last Known Value</th>
        </tr>
        <xsl:for-each select="conditions_directory/condition">
          <tr>
            <td align='center'>
              <xsl:value-of select="condition_type"/>
            </td>
            <td align='center'>
              <xsl:value-of select="is_valid"/>
            </td>
            <td align='center'>
              <xsl:value-of select="current_value"/>
            </td>
          </tr>
        </xsl:for-each>
      </table>
    </xsl:template>
</xsl:stylesheet>
<!-- end xsl -->