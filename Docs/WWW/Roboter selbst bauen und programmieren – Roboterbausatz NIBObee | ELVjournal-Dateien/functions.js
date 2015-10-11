function WaterMark(text, feld, option)
{
 if (option == 'enter')
 {
  if (feld.value == text)
  {
   feld.value = '';
  }
 }

 if (option == 'exit')
 {
  if (feld.value == '')
  {
   feld.value = text;
  }
 }
}

function submitForm(scope) {
	// Formular ermitteln
	var objFORM = scope;
	while (objFORM) {
		if (objFORM.tagName != 'FORM') {
			objFORM = objFORM.parentNode;
		} else {
			break;
		};
	};
	objFORM.submit();
}
function setFieldValue(Fieldname, Fieldvalue) {
	var Fields = document.getElementsByName(Fieldname);
	for (var i=0; i<Fields.length; i++) {
		Fields[i].selectedIndex=Fieldvalue;
	};
};

function MM_swapImgRestore() { //v3.0
  var i,x,a=document.MM_sr; for(i=0;a&&i<a.length&&(x=a[i])&&x.oSrc;i++) x.src=x.oSrc;
}

function MM_preloadImages() { //v3.0
  var d=document; if(d.images){ if(!d.MM_p) d.MM_p=new Array();
	var i,j=d.MM_p.length,a=MM_preloadImages.arguments; for(i=0; i<a.length; i++)
	if (a[i].indexOf("#")!=0){ d.MM_p[j]=new Image; d.MM_p[j++].src=a[i];}}
}

function MM_findObj(n, d) { //v4.01
  var p,i,x;  if(!d) d=document; if((p=n.indexOf("?"))>0&&parent.frames.length) {
	d=parent.frames[n.substring(p+1)].document; n=n.substring(0,p);}
  if(!(x=d[n])&&d.all) x=d.all[n]; for (i=0;!x&&i<d.forms.length;i++) x=d.forms[i][n];
  for(i=0;!x&&d.layers&&i<d.layers.length;i++) x=MM_findObj(n,d.layers[i].document);
  if(!x && d.getElementById) x=d.getElementById(n); return x;
}

function MM_swapImage() { //v3.0
  var i,j=0,x,a=MM_swapImage.arguments; document.MM_sr=new Array; for(i=0;i<(a.length-2);i+=3)
   if ((x=MM_findObj(a[i]))!=null){document.MM_sr[j++]=x; if(!x.oSrc) x.oSrc=x.src; x.src=a[i+2];}
}

function NL_eintragen(nl_anrede, nl_vorname, nl_name, nl_email, nl_geb_tag, nl_geb_monat, nl_geb_jahr){
window.location.href = "http://tre.emv3.com/DUTF8?emv_webformtype=3&emv_bounceback=1&emv_messageid=1100022055&emv_clientid=1100001885&EMAIL_FIELD="+$nl_email+"&FIRSTNAME_FIELD="+$nl_vorname+"&LASTNAME_FIELD="+$nl_name+"&DATEOFBIRTH_FIELD="+$nl_geb_monat+'/'+$nl_geb_tag+'/'+$nl_geb_tag+"";
}

function tvView (url) 
{
	fenster = window.open(url, "tvView", "width=900,height=700,status=no,scrollbars=no,resizable=yes");
	fenster.focus();
}
function checkQuickpoll(objField, msg)
{
	var keine_auswahl = true;
	for (i=0;i<objField.length;i++)
	{
		if (objField[i].checked) keine_auswahl = false;
	}
	if (keine_auswahl)
	{
		alert(msg);
		return false;
	}
	return true;
}
function show(sRequestText) {
  ajaxObjectHitlist.update(sRequestText);
}
function chkEntry(){
	if (jQuery('.suggest_zeile_on').length > 0) {
		jQuery('.suggest_zeile_on').click();
		return(false);
	}
	else{
		return (true)
	}
}

function sendFormFromFlash(article_nr, swfszenario){
	window.document.FORMWKEXT.action = window.document.FORMWKEXT.action+'&swfszenario='+swfszenario;
	var artikelnummer = 'wk_art_'+article_nr;
	var myInput = document.createElement("input");
	myInput.setAttribute("name",artikelnummer);
	myInput.setAttribute("value","1");
	myInput.setAttribute("type","hidden");
	window.document.FORMWKEXT.appendChild(myInput);
	window.document.FORMWKEXT.submit();
}

function hidehmlayer(){
	var ablauf = new Date();
	var zeitpunkt = ablauf.getTime() + (1 * 24 * 60 * 60 * 1000);
	ablauf.setTime(zeitpunkt);

	cookiestring = document.cookie;
	cookiename = cookiestring.substr(0,cookiestring.search('='));
	cookiewert = cookiestring.substr(cookiestring.search('=')+1,cookiestring.search(';'));
	
	document.cookie = "Layer=true; expires=" + ablauf.toGMTString() + ";path=/";
	document.getElementById("slideWrapper").style.right= "-830px";
}

function bkopwin(url){
	var wstat
	var ns4up = (document.layers) ? 1 : 0
	var ie4up = (document.all) ? 1 : 0
	var xsize = screen.width
	var ysize = screen.height
	var breite=1000
	var hoehe=680
	var xpos=(xsize-breite)/2
	var ypos=(ysize-hoehe)/2
	
	wstat=window.open(url,"","scrollbars=no,status=no,toolbar=no,location=no,directories=no,resizable=no,menubar=no,width="+breite+",height="+hoehe+",screenX="+xpos+",screenY="+ypos+",top="+ypos+",left="+xpos)
}
	function nameDefined(ckie,nme)
	{
	   var splitValues
	   var i
	   for (i=0;i<ckie.length;++i)
	   {
		  splitValues=ckie[i].split("=")
		  if (splitValues[0]==nme) return true
	   }
	   return false
	}
	function delBlanks(strng)
	{
	   var result=""
	   var i
	   var chrn
	   for (i=0;i<strng.length;++i) {
		  chrn=strng.charAt(i)
		  if (chrn!=" ") result += chrn
	   }
	   return result
	}
	function getCookieValue(ckie,nme)
	{
	   var splitValues
	   var i
	   for(i=0;i<ckie.length;++i) {
		  splitValues=ckie[i].split("=")
		  if(splitValues[0]==nme) return splitValues[1]
	   }
	   return ""
	}
	function testCookie(cname, cvalue) {
	   var cookie=document.cookie
	   var chkdCookie=delBlanks(cookie)
	   var nvpair=chkdCookie.split(";")
	   if(nameDefined(nvpair,cname))
	   {   
		  tvalue=getCookieValue(nvpair,cname)
		  if (tvalue == cvalue) return true
		   else return false
	   }
	   else return false
	}
		function reDir(){
		switch (window.location.hostname){
			case "www.elv.de":
				window.location.href = 'http://www.elv.de/controller.aspx?cid=997';
				break;
			case "www.elv.at":
				window.location.href = 'http://www.elv.at/controller.aspx?cid=997';
				break;
			case "www.elv.ch":
				window.location.href = 'http://www.elv.ch/controller.aspx?cid=997';
				break;
			default:
				window.location.href = 'http://www.elv.de/controller.aspx?cid=997';
				break;
		}
	}

	function trim (zeichenkette) {
	  return zeichenkette.replace (/^\s+/, '').replace (/\s+$/, '');
	}
	
	function get_url_param( name ){
		name = name.replace(/[\[]/,"\\\[").replace(/[\]]/,"\\\]");
	
		var regexS = "[\\?&]"+name+"=([^&#]*)";
		var regex = new RegExp( regexS );
		var results = regex.exec( window.location.href );
	
		if ( results == null )
			return "";
		else
			return results[1];
	}


	function top_warenkorb_layer_open(obj) {
		if (obj) {
			var formular = obj;
			// formular ermittlen
			while (formular) {
				if (formular.tagName != 'FORM') {
					formular = formular.parentNode;
				}
				else {
					formular.setAttribute('target', 'iframe_warenkorb');
					var formular_action = formular.getAttribute('action');
					if (formular_action.indexOf('&print=8') == -1)
						formular.setAttribute('action', formular_action + '&print=8');
					break;
				}
			}
		};
		window.scrollTo(0, 0);
		document.getElementById('warenkorb_layer').style.display = '';
		return false;
	};

	function top_warenkorb_layer_close() {
		document.getElementById('iframe_warenkorb').src = '/warenkorb_layer_leer.htm';
		document.getElementById('warenkorb_layer').style.display = 'none';
		return false;
	};

	function top_warenkorb_layer_setValue(value) {
		document.getElementById('top_warenkorb').innerHTML = value;
		return true;
	};

	function top_warenkorb_layer_resize(height) {
		if (!isNaN(height))
			document.getElementById('iframe_warenkorb').style.height = height + 'px';
	};

	function top_warenkorb_layer_loadURL(value) {
		document.getElementById('iframe_warenkorb').src = value;
		window.scrollTo(0, 0);
		document.getElementById('warenkorb_layer').style.display = '';

	};
