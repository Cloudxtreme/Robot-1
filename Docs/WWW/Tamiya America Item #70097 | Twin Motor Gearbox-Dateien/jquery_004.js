jQuery.fn.gzoom = function(settings) {
	settings = jQuery.extend({
		sW: 10, // small image width
		sH: 10, // small image height
		lW: 20, // large image width
		lH: 20, // large image height
		step : 5,
		frameColor: "#cecece",
		frameWidth: 1,
		re : /thumbs\//, 
		replace : '',  
		debug : false,
		overlayBgColor : '#000',
		overlayOpacity:0.8,
		containerBorderSize: 10, 
		containerResizeSpeed : 400,
		loaderContent: "loading...",  // plain text or an image tag eg.: "<img src='yoursite.com/spinner.gif' />"
		lightbox: false,
		zoomIcon : "" // icon url, if not empty shown on the right-top corner of the image
	}, settings);

	return this.each(function(){
		var swapped = false;
	  	var $div = jQuery(this).css({width: 946, overflow:'hidden'}).addClass("minizoompan");
		$div.wrap('<div class="gzoomwrap"></div>').css({ width : 946});
		var ig = $div.children().css({position: "relative"});
		ig.css({'visibility': "hidden"});
		jQuery(window).bind("load", function() {
			ig.width(settings.sW); 
			ig.height(settings.sH); 
			ig.css({'visibility': "visible"});
		});
			
		var $plus = $('#galleria-zoom-in');
		var $minus = $('#galleria-zoom-out');
		var $resize = $('#galleria-zoom-resize');
		valore = 1;
		
		$resize.click(function() {
		  	valore = 1;
			ig.css({ width: settings.sW, height: settings.sH});
		  	var divWidth = $div.width();
			var divHeight = $div.height();
			var igW = ig.width();
			var igH = ig.height();
			
			var leftPan = (divWidth - igW) / 2;
			ig.css({left: leftPan});
				
			var topPan = (divHeight - igH) / 2;
			ig.css({top: topPan});
		});
		
		$plus.click(function() {
		  valore = parseInt(valore) + settings.step;
		  zooom(valore);
		});
		
		$minus.click(function() {
		  valore = parseInt(valore) - settings.step;
		  zooom(valore);
		});
		
		var divWidth = $div.width();
		var divHeight = $div.height();
		var igW = settings.sW;
		var igH = settings.sH;
		var leftPan = (divWidth - igW) / 2;
		ig.css({left: leftPan});
			
		var topPan = (divHeight - igH) / 2;
		ig.css({top: topPan});
		
		function zooom(val) {
			newWidth = settings.sW + ((settings.lW - settings.sW)*val)/100;
			newHeight = settings.sH + ((settings.lH - settings.sH)*val)/100;
			ig_pos = ig.position();
			ig.css({ width: newWidth, height: newHeight});
			
			var divWidth = $div.width();
			var divHeight = $div.height();
			var igW = ig.width();
			var igH = ig.height();
			
			var leftPan = (divWidth - igW) / 2;
			ig.css({left: leftPan});
				
			var topPan = (divHeight - igH) / 2;
			ig.css({top: topPan});
		}
		
		$div.mousemove(function(e){
			var divWidth = $div.width();
			var divHeight = $div.height();
			var igW = ig.width();
			var igH = ig.height();
			var dOs = $div.offset();
			if(divWidth < igW) {
				var leftPan = (e.pageX - dOs.left) * (divWidth - igW) / (divWidth+settings.frameWidth*2);
				ig.css({left: leftPan});	
			} else {
				var leftPan = (divWidth - igW) / 2;
				ig.css({left: leftPan});
			}
			if(divWidth < igW ||divHeight <  igH) {
				var topPan = (e.pageY - dOs.top) * (divHeight - igH) / (divHeight+settings.frameWidth*2);
				ig.css({top: topPan});	
			} else {
				var topPan = (divHeight - igH) / 2;
				ig.css({top: topPan});
			}
		});
	});	
};