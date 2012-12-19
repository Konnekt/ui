<?xml version="1.0" encoding="ISO-8859-2"?>
<xsl:stylesheet 
    version="1.0" 
    xmlns:msxsl="urn:schemas-microsoft-com:xslt"
    xmlns:stamina="http://www.stamina.eu.org/"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output method="html"/>
<xsl:template match="//history">
<html>
<title>Konnekt: Historia [ <xsl:value-of select="info/profile/." disable-output-escaping="yes"/> | <xsl:value-of select="info/date/." disable-output-escaping="yes"/> ]</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-2" />
<style type="text/css">
  body {
    font-family:verdana, tahoma, arial;
    font-size:11px;
    text-align: left;
    margin: 15px 15px 15px 15px;
    background-color:white;
  }
  div {
    padding: 5px 0px 5px 5px
  }
  
  .title {
  font-size:20px;
  font-weight: bold;
  width: 100%;
  background-color:#eeeeee;
  }
  
  .mes {
  font-size:12px;
  }
  
  .dir{
  padding: 5px 20px 5px 20px;
	background-color: #eeeeee;
  }
  .contact {
	background-color: #eedddd;
	}
	.message {
	background-color: #ddeedd;
	}
	.frommessage {
	background-color: #ccddcc;
	}
	.chat {
	background-color: #ddcccc;
	}

</style>

<body>
<div class="title">Konnekt: Historia</div>
<br />
<div class="dir"><xsl:apply-templates /></div>
</body>
</html>
</xsl:template>

<xsl:template match="dir">
  <div class="dir">
  <b><font class="big"><xsl:value-of select="@name"/></font></b> (<xsl:value-of select="@info"/>):
  <xsl:apply-templates select="contact|chat"/>
  </div>
</xsl:template>

<xsl:template match="contact">
  <div class="dir contact">
	<br/>
  <b><u>Rozmowy z <font color="red"><xsl:value-of select="@name"/></font></u>:</b>
	<br/><br/>
  <xsl:apply-templates select="message"/>
  <xsl:apply-templates select="chat"/>
  <hr noshade="noshade" size="1px" />
</div>
</xsl:template>

<xsl:template match="chat">
  <div class="dir chat">
  <b><u>Rozmowa z dnia <xsl:value-of select="@date"/> / <xsl:value-of select="@time"/></u></b><br />
  <xsl:apply-templates select="message"/>
  </div>
</xsl:template>

<xsl:template match="message">
  <xsl:choose>
	<xsl:when test="@toUid=''">
    <xsl:value-of select="string('&lt;div class=&quot;dir frommessage&quot;&gt;')" disable-output-escaping="yes"/>
	</xsl:when>
  <xsl:otherwise>
  <xsl:value-of select="string('&lt;div class=&quot;dir message&quot;&gt;')" disable-output-escaping="yes"/>
	</xsl:otherwise>	  
  </xsl:choose>	
  <b><xsl:value-of select="@name"/></b> (<xsl:value-of select="@date"/> / <xsl:value-of select="@time"/>)<br />
  <font class="mes">
  <xsl:value-of select="stamina:ln2br(string(.))" disable-output-escaping="yes"/>
  </font>
  <xsl:value-of select="'&lt;/div&gt;'" disable-output-escaping="yes"/>
  
</xsl:template>

<xsl:template match="info">
<div>
data exportu: <b><xsl:value-of select="date"/></b><br />
profil: <b><xsl:value-of select="profile"/></b><br />
</div>
</xsl:template>

  <msxsl:script language="JScript" implements-prefix="stamina">
     function ln2br(text) {
        return text.replace(/\r\n|\n/g , "&lt;br/&gt;");
     }
  </msxsl:script>

</xsl:stylesheet>