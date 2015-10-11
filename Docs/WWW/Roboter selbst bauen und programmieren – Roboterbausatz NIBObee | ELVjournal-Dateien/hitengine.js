function ajaxObject(sUrl, fnReadyStateComplete, fnResponseProcess, oElement) {
	var that = this;

	this.updating = false;
	this.abort = function() {
		if (that.updating) {
			that.updating = false;
			if (that.interval) {
				window.clearInterval(that.interval);
				that.interval = null;
			}
			that.AJAX.abort();
			that.AJAX = null;
		}
	}
	this.interval = null;
	this.clearInterval = function() {
		if (that.interval) {
			window.clearInterval(that.interval);
			that.interval = null;
		}
	}
	this.update = function(passData,postMethod,syncMethod) {
		var oUpdateElementLocale = (that.update.arguments.length > 3 ? that.update.arguments[3] : oUpdateElement);

		if (that.updating) { return false; }
		try {
			that.AJAX = new ActiveXObject("Msxml2.XMLHTTP");
		} catch(e) {
			try{
				that.AJAX = new ActiveXObject("Microsoft.XMLHTTP");
			} catch(sc) {
				that.AJAX = null;
			}
		}
		if (!that.AJAX && typeof XMLHttpRequest!="undefined") {
			that.AJAX = new XMLHttpRequest();
		}
		if (that.AJAX == null) {
			return false;
		} else {
			if (!(/^sync/i.test(syncMethod))) {
				that.AJAX.onreadystatechange = function() {
					if (that.AJAX.readyState == 4) {
						that.updating = false;
						that.callback(that.AJAX.responseText,that.AJAX.status,that.AJAX.responseXML,that.requestText,that.callbackCustom,oUpdateElementLocale);
						that.AJAX = null;
					}
				}
			}
			that.updating = new Date();
			that.requestText = passData;
			if (/^post/i.test(postMethod)) {
				var sTimestamp = that.updating.getTime();
				var sUri = sUrl + '?' + sTimestamp;
				that.AJAX.open("POST", sUri, !(/^sync/i.test(syncMethod)));
				that.AJAX.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
				that.AJAX.setRequestHeader("Content-length", passData.length);
				that.AJAX.setRequestHeader("Connection", "close");
				that.AJAX.send(passData + '&host=' + window.location.host);
			} else {
				var sTimestamp = that.updating.getTime();
				var sUri = sUrl + '?' + passData + '&host=' + window.location.host + '&timestamp=' + sTimestamp;
				that.AJAX.open("GET", sUri, !(/^sync/i.test(syncMethod)));
				that.AJAX.send(null);
			}
			if (/^sync/i.test(syncMethod)) {
				that.updating = false;
				that.callback(that.AJAX.responseText,that.AJAX.status,that.AJAX.responseXML,that.requestText,that.callbackCustom,oUpdateElementLocale);
				that.AJAX = null;
			}
			return true;
		}
	}
	var oUpdateElement  = oElement || null;
	this.callback       = fnReadyStateComplete || function () { };
	that.callbackCustom = fnResponseProcess || function () { };
}

function ajaxCallback(sResponseText, iStatus, sResponseXML, sRequestText, fnResponseProcess, oUpdateElement) {
	if (iStatus == 200) {
		fnResponseProcess(sResponseText, sRequestText, oUpdateElement);
	}
}

function evalHitEngine(sResponseText) {
	var iPosStart = -1;
	var iPosEnd   = -1;

	jQuery('#searchoutputbox').css('display', 'none');

	if (sResponseText.search(/<result>/i) == -1) {
		jQuery('#searchoutputbox').html(sResponseText);
		jQuery('#searchoutputbox').css('display', 'block');

		jQuery('.suggest_zeile').hover(function() {
			jQuery('.suggest_zeile_on').removeClass('suggest_zeile_on');
			jQuery(this).addClass('suggest_zeile_on');
		}, function() {
			jQuery(this).removeClass('suggest_zeile_on');
		});

		var iSelectedGroup = (jQuery('#search').attr('selectedgroup') ? jQuery('#search').attr("selectedgroup") : -1);

		jQuery('#search').removeAttr('selectedgroup');
		if (iSelectedGroup > -1) {
			jQuery('.suggest_group_' + iSelectedGroup + ':first').addClass('suggest_zeile_on');
		}
	}
	else {
		iPosStart = sResponseText.search(/<debug>/i);
		if (iPosStart != -1) {
			iPosStart = iPosStart + "<debug>".length;
			iPosEnd   = sResponseText.search(/<\/debug>/i);

			if (iPosEnd > iPosStart) {
				jQuery('#hitengine_message').html(sResponseText.substring(iPosStart, iPosEnd));
			}
		}
		iPosStart = sResponseText.search(/<comps>/i);
		if (iPosStart != -1) {
			iPosStart = iPosStart + "<comps>".length;
			iPosEnd   = sResponseText.search(/<\/comps>/i);
			if (iPosEnd >= iPosStart) {
				var sComps = "";

				sComps = sResponseText.substring(iPosStart, iPosEnd);
				sComps = sComps.replace(/^\s*/g, "");
				sComps = sComps.replace(/\s*$/g, "");

				jQuery('#searchoutputbox').html(sComps);
				jQuery('#searchoutputbox').css('display', 'block');

				jQuery('.suggest_zeile').hover(function() {
					jQuery('.suggest_zeile_on').removeClass('suggest_zeile_on');
					jQuery(this).addClass('suggest_zeile_on');
				}, function() {
					jQuery(this).removeClass('suggest_zeile_on');
				});
				
				var iSelectedGroup = (jQuery('#search').attr('selectedgroup') ? jQuery('#search').attr("selectedgroup") : -1);

				jQuery('#search').removeAttr('selectedgroup');
				if (iSelectedGroup > -1) {
					jQuery('.suggest_group_' + iSelectedGroup + ':first').addClass('suggest_zeile_on');
				}
			}
		}
	}
}

function showHitEngine(sRequestText) {
	ajaxObjectHitEngine.update(sRequestText, "POST");
}
var ajaxObjectHitEngine = new ajaxObject('/ajax_functions/autocomplete_suche_wk.aspx', ajaxCallback, evalHitEngine);
jQuery.noConflict();
jQuery(document).ready(function(){
	jQuery('#search').bind('keyup', function() {
		var sValue = jQuery.trim(jQuery(this).val());
		var sValueLast = jQuery(this).attr("valuelast");
		var sRequestText = "search=" + encodeURIComponent(sValue);

		if (sValue != "" && isNaN(sValue) &&
		    sValue != sValueLast) {
			ajaxObjectHitEngine.clearInterval();                    // Initialisierte Timer beenden
			ajaxObjectHitEngine.interval = window.setInterval(function() {
				ajaxObjectHitEngine.abort();                        // Laufende Requests abbrechen
				ajaxObjectHitEngine.update(sRequestText, "POST");   // Request absetzen
				ajaxObjectHitEngine.clearInterval();                // Wenn Request abgesetzt, dann Timer beenden
			}, 250);
			jQuery(this).attr("valuelast", sValue);
		}
	}).bind('keydown', function(event) {
		var sSelected      = "on";  // oder 'selected'
		var iSelected      = jQuery('.' + 'suggest' + '_zeile_' + sSelected).index('.' + 'suggest' + '_zeile');
		var iNumber        = jQuery('.' + 'suggest' + '_zeile').length;
		var iSelectedGroup = jQuery('.' + 'suggest' + '_zeile_' + sSelected).map(function(){return (jQuery(this).attr('class').match(/suggest_group_(\d+)/) ? RegExp.$1 : -1)}).get(0);

		if (event.keyCode == 40) {       // Falls es sich um Cursor nach unten handelt
			var iSelectedNew = (iSelected >= iNumber - 1 ? 0 : iSelected + 1);

			if (iNumber > 0 && jQuery('#searchoutputbox').css('display') == 'none') {
				jQuery('#searchoutputbox').css('display', 'block');
			}
			jQuery('.' + 'suggest' + '_zeile_' + sSelected).removeClass('suggest' + '_zeile_' + sSelected);
			jQuery('.' + 'suggest' + '_zeile:eq(' + iSelectedNew + ')').addClass('suggest' + '_zeile_' + sSelected);
		}
		else if (event.keyCode == 38) {  // Falls es sich um Cursor nach oben handelt
			var iSelectedNew = (iSelected <= 0 ? iNumber - 1 : iSelected - 1);
			
			jQuery('.' + 'suggest' + '_zeile_' + sSelected).removeClass('suggest' + '_zeile_' + sSelected);
			jQuery('.' + 'suggest' + '_zeile:eq(' + iSelectedNew + ')').addClass('suggest' + '_zeile_' + sSelected);
		}
		else if (event.keyCode == 13) {  // Falls RETURN in der Suggest-Liste gedrückt wurde
			if (jQuery('.' + 'suggest' + '_zeile_' + sSelected).length > 0) {
				jQuery('.' + 'suggest' + '_zeile_' + sSelected).click();
			}
		}
		else if (event.keyCode == 27) {  // Falls ESC in der Suggest-Liste gedrückt wurde
			jQuery('#searchoutputbox').css('display', 'none');
		}
		else if (event.keyCode == 33) {  // Falls BILD OBEN in der Suggest-Liste gedrückt wurde
			if (iSelectedGroup > -1) jQuery(this).attr("selectedgroup", iSelectedGroup);
			jQuery('.' + 'suggest' + '_zeile_' + sSelected + ' ~ .' + 'suggest' + '_nav_zeile').find('span').map(function(){return (jQuery(this).css('visibility') == 'hidden' ? null : this)}).find('a.' + 'suggest' + '_hitprev').click();
		}
		else if (event.keyCode == 34) {  // Falls BILD UNTEN in der Suggest-Liste gedrückt wurde
			if (iSelectedGroup > -1) jQuery(this).attr("selectedgroup", iSelectedGroup);
			jQuery('.' + 'suggest' + '_zeile_' + sSelected + ' ~ .' + 'suggest' + '_nav_zeile').find('span').map(function(){return (jQuery(this).css('visibility') == 'hidden' ? null : this)}).find('a.' + 'suggest' + '_hitnext').click();
		}
	});
});


