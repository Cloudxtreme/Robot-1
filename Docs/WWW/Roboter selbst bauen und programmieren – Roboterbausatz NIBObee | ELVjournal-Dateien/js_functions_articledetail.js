/*
function sendEvent(typ,prm) { thisMovie("ply").sendEvent(typ,prm); };
function thisMovie(movieName) {
	if(navigator.appName.indexOf("Microsoft") != -1) {
		return window[movieName];
	} else {
		return document[movieName];
	}
};
*/
function showVideo() {
	// Aktives DIV setzen
	if (document.getElementById('div_article_pic').style.display != 'none')
		document.getElementById('div_article_pic').style.display = 'none';
//	if (document.getElementById('div_article_video').style.display == 'none')
//		document.getElementById('div_article_video').style.display = '';
//		document.getElementById('ply').focus();
}
function showPic(pic_src, title) {
	// Video pausieren
	if (document.getElementById('ply')) {
		document.getElementById('ply').sendEvent('PLAY', 'false');
	}

	// Aktives DIV setzen
//	if (document.getElementById('div_article_video').style.display != 'none')
//		document.getElementById('div_article_video').style.display = 'none';
	if (document.getElementById('div_article_pic').style.display == 'none')
		document.getElementById('div_article_pic').style.display = '';

	// Bild tauschen
	document.getElementById('article_pic').src = pic_src;

	// Titel setzen
	if (title.length != 0) {
		document.getElementById('article_pic_title').innerHTML = title;
		document.getElementById('article_pic_title').style.display = 'block';
	}
	else {
		document.getElementById('article_pic_title').innerHTML = '';
		document.getElementById('article_pic_title').style.display = 'none';
	}
}

function openPicPopup(art_id, bild_id) {
	var url = '/module/produktkatalog01/bilder_popup.aspx?art_id=' + art_id
	if (bild_id != '') url = url + '&amp;bild_id=' + bild_id;
	window.open(url, 'bild_popup', 'width=555, height=600')
	return false;
}

function setTab(id) {
	for (var i = 1; i < 11; i++) {
		if (document.getElementById('tab' + i) && i != 9) document.getElementById('tab' + i).className = 'tab';
		document.getElementById('tab_content' + i).style.display = 'none';
	}
	if (document.getElementById('tab' + id) && id != 9) document.getElementById('tab' + id).className = 'tab_aktiv';
	document.getElementById('tab_content' + id).style.display = '';
}

function initTabs() {
	for (var i = 1; i < 11; i++) {
		if (document.getElementById('tab' + i))
		{
			document.getElementById('tab' + i).className = 'tab_aktiv'
			document.getElementById('tab_content' + i).style.display = '';
			break;
		}
	}
}

/* Slider */
var activeTimer;
var scrollWidth = 5;
var scrollTimeout = 50;

function scroll(id, direction, tmpLeft) {

	var stop = false;

	var objDIV_Slider = document.getElementById('slider_' + id);
	var objIMG_Slider_Left = document.getElementById('left_slider_' + id);
	var objIMG_Slider_Right = document.getElementById('right_slider_'+id);

	// Scroll-Vorgang
	if (direction != 'right') {
		objDIV_Slider.scrollLeft = objDIV_Slider.scrollLeft - scrollWidth;
	}
	else {
		objDIV_Slider.scrollLeft = objDIV_Slider.scrollLeft + scrollWidth;
	}

	// Pfeil sichtbar oder unsichtbar
	if (objDIV_Slider.scrollLeft == 0) {
		objIMG_Slider_Left.style.display = 'none';
		stop = true;
	}
	else {
		objIMG_Slider_Left.style.display = '';
	}

	if (tmpLeft == objDIV_Slider.scrollLeft) {
		objIMG_Slider_Right.style.display = 'none';
		stop = true;
	}
	else {
		objIMG_Slider_Right.style.display = '';
		tmpLeft = objDIV_Slider.scrollLeft;
	}

	if (!stop) activeTimer = setTimeout("scroll('" + id + "', '" + direction + "', " + tmpLeft + ")", scrollTimeout);

}

function scroll2(id, direction, tmpTop) {

	var stop = false;

	var objDIV_Slider = document.getElementById('slider_' + id);
	var objIMG_Slider_Top = document.getElementById('top_slider_' + id);
	var objIMG_Slider_Bottom = document.getElementById('bottom_slider_'+id);

	// Scroll-Vorgang
	if (direction != 'bottom') {
		objDIV_Slider.scrollTop = objDIV_Slider.scrollTop - scrollWidth;
	}
	else {
		objDIV_Slider.scrollTop = objDIV_Slider.scrollTop + scrollWidth;
	}

	// Pfeil sichtbar oder unsichtbar
	if (objDIV_Slider.scrollTop == 0) {
		objIMG_Slider_Top.style.display = 'none';
		stop = true;
	}
	else {
		objIMG_Slider_Top.style.display = '';
	}

	if (tmpTop == objDIV_Slider.scrollTop) {
		objIMG_Slider_Bottom.style.display = 'none';
		stop = true;
	}
	else {
		objIMG_Slider_Bottom.style.display = '';
		tmpTop = objDIV_Slider.scrollTop;
	}

	if (!stop) activeTimer = setTimeout("scroll2('" + id + "', '" + direction + "', " + tmpTop + ")", scrollTimeout);

}

function scroll_stop() {
	clearTimeout(activeTimer);
}
/* Ende: Slider */

/* Artikellogos */
function showLogo(obj, direction) {
	document.getElementById(obj.id + '_layer').style.display = '';
	
}

function hideLogo(obj) {
	obj.style.display = 'none';
}

	// Support ANF-1504-XDK6HS - enobis GmbH - ML
	function classShowPopup(klein_id, gross_id) {
		var instance = this;
		this.div_klein = document.getElementById(klein_id);
		this.div_gross = document.getElementById(gross_id);
		this.timer = null;

		this.show = function() {
			if (this.div_gross.style.display == 'none') this.div_gross.style.display = 'block';
		};

		this.hide = function() {
			if (this.div_gross.style.display == 'block') this.div_gross.style.display = 'none';
			window.clearInterval(instance.timer);
		}

		this.MouseOver = function() {
			window.clearInterval(instance.timer);
			instance.show();
			instance.timer = window.setInterval(function() { instance.hide() }, 3000);
		}

		this.MouseOut = function() {
			instance.hide();
		}

		this.div_klein.onmouseover = this.MouseOver;
		this.div_gross.onmousemove = this.MouseOver;
		this.div_gross.onmouseout = this.MouseOut;

	};

/* Ende: Artikellogos */