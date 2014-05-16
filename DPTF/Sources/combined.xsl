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
            <xsl:when test="active_control/active_control_status">
              <td>
                <xsl:value-of select="../../participant_properties/description" />&#160;
                (<xsl:value-of select="active_control/active_control_status/current_control_id" />)
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
      <td align='right'><xsl:value-of select="temperature_control/temperature_status" /></td>

      <xsl:choose>
      <xsl:when test="temperature_control/temperature_thresholds">
        <td align='right'><xsl:value-of select="temperature_control/temperature_thresholds/aux0" /></td>
        <td align='right'><xsl:value-of select="temperature_control/temperature_thresholds/aux1" /></td>
        <td align='right'><xsl:value-of select="temperature_control/temperature_thresholds/hysteresis" /></td>
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
  <xsl:for-each select="callback_scheduler/target_scheduler/target_callback">
  <tr>
    <td><xsl:value-of select="target"/></td>
    <td align='right'><xsl:value-of select="time_until_expires"/></td>
  </tr>
  </xsl:for-each>
</table>

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
  <tr bgcolor="#00AEEF" colspan="5">
    <th colspan="5">Current Cooling Preference</th>
  </tr>
  <tr bgcolor="#00AEEF">
    <th>Name</th>
    <th>Value</th>
  </tr>
  <tr>
    <td align='left'>Cooling Mode</td>
    <td align='left'><xsl:value-of select="cooling_preference/cooling_mode"/></td>
  </tr>
  <tr>
    <td align='left'>Acoustic Limit</td>
    <td align='left'><xsl:value-of select="cooling_preference/acoustic_limit"/></td>
  </tr>
  <tr>
    <td align='left'>Power Limit</td>
    <td align='left'><xsl:value-of select="cooling_preference/power_limit"/></td>
  </tr>
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
    <th>Connection Status</th>
    <th>Center Frequency</th>
    <th>Left Spread</th>
    <th>Right Spread</th>
  </tr>
  <xsl:for-each select="radio_device_list/radio_device">
  <tr>
    <td align='left'><xsl:value-of select="participant_name"/> (<xsl:value-of select="participant_index"/>)</td>
    <td align='left'><xsl:value-of select="domain_name"/> (<xsl:value-of select="domain_index"/>)</td>
    <td align='right'><xsl:value-of select="last_known_connection_status"/></td>
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
  <xsl:if test="domain_priority/domain_priority">
    <table border="1">
    <tr bgcolor="#00AEEF" colspan="2">
      <th colspan="2">Domain Properties</th>
    </tr>
    <tr>
      <td>Priority</td>
      <td align='right'><xsl:value-of select="domain_priority/domain_priority" /></td>
    </tr>
    </table>
    <br></br>
  </xsl:if>
  <!-- END - Domain Priority -->

  <!-- Temperature Control -->
  <xsl:if test="temperature_control">
    <table border="1">
    <tr bgcolor="#00AEEF" colspan="4">
      <th colspan="4">Temperature Control</th>
    </tr>
    <tr bgcolor="#00AEEF">
      <th>Temperature</th>
      <xsl:if test="temperature_control/temperature_thresholds">
        <th>Aux 0</th>
        <th>Aux 1</th>
        <th>Hysteresis</th>
      </xsl:if>
    </tr>
    <tr>
      <td align='right'><xsl:value-of select="temperature_control/temperature_status" /></td>
      <xsl:if test="temperature_control/temperature_thresholds">
        <td align='right'><xsl:value-of select="temperature_control/temperature_thresholds/aux0" /></td>
        <td align='right'><xsl:value-of select="temperature_control/temperature_thresholds/aux1" /></td>
        <td align='right'><xsl:value-of select="temperature_control/temperature_thresholds/hysteresis" /></td>
      </xsl:if>
    </tr>
    </table>
    <br></br>
  </xsl:if>
  <!-- END - Temperature Control -->

  <!-- Performance Control -->
  <!-- Performance Control - Status -->
  <xsl:if test="performance_control">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="2">
        <th colspan="2">Performance Control Status</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Current Index</th>
      </tr>
      <tr>
        <td align='right'><xsl:value-of select="performance_control/performance_control_status/current_index" /></td>
      </tr>
    </table>
    <br></br>
  </xsl:if>
  <!-- END - Performance Control - Status -->

  <!-- Performance Control - Static Caps -->
  <xsl:if test="performance_control">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="2">
        <th colspan="2">Performance Control Static Caps</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Dynamic Performance Control States</th>
      </tr>
      <tr>
        <td><xsl:value-of select="performance_control/performance_control_static_caps/dynamic_performance_control_states" /></td>
      </tr>
    </table>
    <br></br>
  </xsl:if>
  <!-- END - Performance Control - Static Caps -->

  <!-- Performance Control - Dynamic Caps -->
  <xsl:if test="performance_control">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="2">
        <th colspan="2">Performance Control Dynamic Caps</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Upper Limit</th>
        <th>Lower Limit</th>
      </tr>
      <tr>
        <td align='right'><xsl:value-of select="performance_control/performance_control_dynamic_caps/upper_limit_index" /></td>
        <td align='right'><xsl:value-of select="performance_control/performance_control_dynamic_caps/lower_limit_index" /></td>
      </tr>
    </table>
    <br></br>
  </xsl:if>
  <!-- END : Performance Control - Dynamic Caps -->

  <!-- Performance Control Set -->
  <xsl:if test="performance_control">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="7">
        <th colspan="7">Performance Control Set</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Index</th>
        <th>Control ID</th>
        <th>Type</th>
        <th>TDP Power (mW)</th>
        <th>Performance %</th>
        <th>Transition Latency (ms)</th>
        <th>Control Value</th>
      </tr>
      <xsl:for-each select="performance_control/performance_control_set/performance_control">
      <tr>
        <td align='right'><xsl:value-of select="position() - 1" /></td>
        <td align='right'><xsl:value-of select="control_id" /></td>
        <td><xsl:value-of select="control_type" /></td>
        <td align='right'><xsl:value-of select="tdp_power" /></td>
        <td align='right'><xsl:value-of select="performance_percentage" /></td>
        <td align='right'><xsl:value-of select="transition_latency" /></td>
        <td align='right'><xsl:value-of select="control_absolute_value" />&#160;<xsl:value-of select="value_units" /></td>
      </tr>
      </xsl:for-each>
    </table>
    <br></br>
  </xsl:if>
  <!-- END : Performance Control Set -->
  <!-- END : Performance Control -->

  <!-- Power Status -->
  <xsl:if test="power_status">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="7">
        <th colspan="7">Power Status</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Power</th>
      </tr>
      <tr>
        <td align='right'><xsl:value-of select="power_status/power" /></td>
      </tr>
    </table>
    <br></br>
  </xsl:if>
  <!-- END : Power Status -->

  <!-- Power Control Status -->
  <xsl:if test="power_control/power_control_status_set">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="4">
        <th colspan="4">Power Control Status</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Control Type</th>
        <th>Power Limit (mW)</th>
        <th>Time Window</th>
        <th>Duty Cycle</th>
      </tr>
      <xsl:for-each select="power_control/power_control_status_set/power_control_status">
      <tr>
        <td><xsl:value-of select="control_type" /></td>
        <td align='right'><xsl:value-of select="power_limit" /></td>
        <td align='right'><xsl:value-of select="time_window" /></td>
        <td align='right'><xsl:value-of select="duty_cycle" /></td>
      </tr>
      </xsl:for-each>
    </table>
    <br></br>
  </xsl:if>
  <!-- END : Power Control Status -->

  <!-- Power Control Dynamic Caps -->
  <xsl:if test="power_control/power_control_dynamic_caps_set">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="8">
        <th colspan="8">Power Control Dynamic Caps</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Control Type</th>
        <th>Max Power Limit (mW)</th>
        <th>Min Power Limit (mW)</th>
        <th>Power Step Size</th>
        <th>Max Time Window (ms)</th>
        <th>Min Time Window (ms)</th>
        <th>Max Duty Cycle</th>
        <th>Min Duty Cycle</th>
      </tr>
      <xsl:for-each select="power_control/power_control_dynamic_caps_set/power_control_dynamic_caps">
      <tr>
        <td><xsl:value-of select="control_type" /></td>
        <td align='right'><xsl:value-of select="max_power_limit" /></td>
        <td align='right'><xsl:value-of select="min_power_limit" /></td>
        <td align='right'><xsl:value-of select="power_step_size" /></td>
        <td align='right'><xsl:value-of select="max_time_window" /></td>
        <td align='right'><xsl:value-of select="min_time_window" /></td>
        <td align='right'><xsl:value-of select="max_duty_cycle" /></td>
        <td align='right'><xsl:value-of select="min_duty_cycle" /></td>
      </tr>
      </xsl:for-each>
    </table>
    <br></br>
  </xsl:if>
  <!-- END : Power Control Dynamic Caps -->

  <!-- Core Control -->
  <xsl:if test="core_control">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="2">
        <th colspan="2">Core Control</th>
      </tr>
      <tr>
        <td>Active Logical Processors</td>
        <td align='right'><xsl:value-of select="core_control/core_control_status/active_logical_processors" /></td>
      </tr>
      <tr>
        <td>Total Logical Processors</td>
        <td align='right'><xsl:value-of select="core_control/core_control_static_caps/total_logical_processors" /></td>
      </tr>
      <tr>
        <td>Max Active Cores</td>
        <td align='right'><xsl:value-of select="core_control/core_control_dynamic_caps/max_active_cores" /></td>
      </tr>
      <tr>
        <td>Min Active Cores</td>
        <td align='right'><xsl:value-of select="core_control/core_control_dynamic_caps/min_active_cores" /></td>
      </tr>
      <tr>
        <td>LPO Enabled</td>
        <td><xsl:value-of select="core_control/core_control_lpo_preference/lpo_enabled" /></td>
      </tr>
      <tr>
        <td>Start P-State</td>
        <td align='right'><xsl:value-of select="core_control/core_control_lpo_preference/start_p_state" /></td>
      </tr>
      <tr>
        <td>Power Offlining Mode</td>
        <td><xsl:value-of select="core_control/core_control_lpo_preference/power_control_offlining_mode" /></td>
      </tr>
      <tr>
        <td>Performance Offlining Mode</td>
        <td><xsl:value-of select="core_control/core_control_lpo_preference/performance_control_offlining_mode" /></td>
      </tr>
    </table>
  </xsl:if>
  <!-- END : Core Control -->

  <!-- Display Control -->
  <xsl:if test="display_control">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="1">
        <th colspan="7">Display Status</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Brightness</th>
      </tr>
      <tr>
        <td align='right'><xsl:value-of select="display_control/display_control_status/brightness_limit_index" /></td>
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
        <td align='right'><xsl:value-of select="display_control/display_control_dynamic_caps/upper_limit_index" /></td>
        <td align='right'><xsl:value-of select="display_control/display_control_dynamic_caps/lower_limit_index" /></td>
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
      <xsl:for-each select="display_control/display_control_set/display_control">
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
  <xsl:if test="active_control">
    <table border="1">
      <tr bgcolor="#00AEEF" colspan="2">
        <th colspan="2">Active Control Status</th>
      </tr>
      <tr bgcolor="#00AEEF">
        <th>Control ID / Fan Speed %</th>
        <th>Speed (RPM) for Control ID</th>
      </tr>
      <tr>
        <td align='right'><xsl:value-of select="active_control/active_control_status/current_control_id" /></td>
        <td align='right'><xsl:value-of select="active_control/active_control_status/current_speed" /></td>
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
        <td><xsl:value-of select="active_control/active_control_static_caps/fine_grained_control" /></td>
        <td><xsl:value-of select="active_control/active_control_static_caps/low_speed_notification" /></td>
        <td align='right'><xsl:value-of select="active_control/active_control_static_caps/step_size" /></td>
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
      <xsl:for-each select="active_control/active_control_set/active_control">
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
