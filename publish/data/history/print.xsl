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
    font-family:"Georgia","Times New Roman",serif;
    text-align: left;
    margin: 5px 5px 5px 5px;
    background-color:white;
  }
  div {
  }
  
  .title {
  font-size:200%;
  font-weight: bold;
	border-bottom:1px solid black;
  }
  
  .mes {	
  }
  
  .dir{
  }
	
	.info {
  font-size:150%;	
	border-bottom:1px solid black;	
	}
	
  .contact { 
	padding: 5px 0px;
	margin:5px 0px;
	border-bottom:1px solid black;	
	}
	
	.message {
	border-left:4px solid red;
	padding:3px;
	margin-bottom:1px;
	}
	.frommessage {
	border-left:4px solid blue;
	padding:3px;
	margin-bottom:1px;		
	}
	.chat {
	padding:10px 0px; 
	border-top:1px solid #aaaaaa;	
	}
  .big {
	font-size: 120%;
	}
</style>

<body onLoad="window.print()">
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
<div class="info">
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