#!/usr/bin/webif-page
<?
. /etc/os-release
. /usr/lib/webif/webif.sh
header "Info" "Router Info" "@TR<<Router Info>>"

?>
<pre><?
_version="${VERSION}"
_kversion="$(uname -a)"
_date="$(date)"
_mac="$(/sbin/ifconfig eth0 | grep HWaddr | cut -b39-)"
sed -e 's/&/&amp;/g' < /etc/motd
cat <<EOF
</pre>
<br />
<table style="width: 90%; text-align: left;" border="0" cellpadding="2" cellspacing="2" align="center">
<tbody>
	<tr>
		<td>@TR<<Firmware Version>></td>
		<td>$_version</td>
	</tr>
	<tr>
		<td>@TR<<Kernel Version>></td>
		<td>$_kversion</td>
	</tr>
	<tr>
		<td>@TR<<Current Date/Time>></td>
		<td>$_date</td>
	</tr>
	<tr>
		<td>@TR<<MAC Address>></td>
		<td>$_mac</td>
	</tr>
</tbody>
</table>
EOF

footer
?>
<!--
##WEBIF:name:Info:10:Router Info
-->
