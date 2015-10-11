$(document).ready(function(){  
/* =============================================================================
   // Checks the browser and adds classes to the body to reflect it.
=============================================================================*/
    var userAgent = navigator.userAgent.toLowerCase();
    $.browser.chrome = /chrome/.test(navigator.userAgent.toLowerCase());    
    // Is this a version of IE?
    if($.browser.msie){
        $('body').addClass('IE');        
        // Add the version number
        if($.browser.version.indexOf('.') != -1){
            userAgent = $.browser.version.split('.');
            userAgent = userAgent[0];
        } else {
            userAgent = $.browser.version.substring(0,1);
        }
        $('body').addClass('IE' + userAgent);
    }  
    // Is this a version of Chrome?
    if($.browser.chrome){   
        $('body').addClass('Chrome');        
        //Add the version number
        userAgent = userAgent.substring(userAgent.indexOf('chrome/') +7);
        if(userAgent.indexOf('.') != -1){
            userAgent = userAgent.split('.');
            userAgent = userAgent[0];
        } else {
            userAgent = userAgent.substring(0,1);
        }
        $('body').addClass('Chrome' + userAgent);        
        // If it is chrome then jQuery thinks it's safari so we have to tell it it isn't
        $.browser.safari = false;
    }    
    // Is this a version of Safari?
    if($.browser.safari){
        $('body').addClass('Safari');       
        // Add the version number
        userAgent = userAgent.substring(userAgent.indexOf('version/') +8);
        if(userAgent.indexOf('.') != -1){
            userAgent = userAgent.split('.');
            userAgent = userAgent[0];
        } else {
            userAgent = userAgent.substring(0,1);
        }
        $('body').addClass('Safari' + userAgent);
    }   
    // Is this a version of Mozilla?
    if($.browser.mozilla){       
        //Is it Firefox?
        if(navigator.userAgent.toLowerCase().indexOf('firefox') != -1){
            $('body').addClass('Firefox');           
            // Add the version number
            userAgent = userAgent.substring(userAgent.indexOf('firefox/') +8);
            if(userAgent.indexOf('.') != -1){
                userAgent = userAgent.split('.');
                userAgent = userAgent[0];
            } else {
                userAgent = userAgent.substring(0,1);
            }
            $('body').addClass('Firefox' + userAgent);
        }
        // If not then it must be another Mozilla
        else{
            $('body').addClass('Mozilla');
        }
    }  
    // Is this a version of Opera?
    if($.browser.opera){
        $('body').addClass('Opera');
    }   
    var OSName = '';
    if (navigator.appVersion.indexOf("Win")!=-1) OSName = "win";
    if (navigator.appVersion.indexOf("Mac")!=-1) OSName = "mac";
    if (navigator.appVersion.indexOf("X11")!=-1) OSName = "unix";
    if (navigator.appVersion.indexOf("Linux")!=-1) OSName = "linux";
    if (OSName  != '') { $('html').addClass(OSName); } 

/* =============================================================================
   Function to Clear Inputs on Click 
=============================================================================*/
	
    $.fn.search = function(value) {
		if(typeof(value) == 'undefined') {
			if($(this).attr('defaultValue') != "") {
				value = $(this).attr('defaultValue');
			}
		}
		if($(this).val() == '' && value != "") {
			$(this).val(value);
		}
        return this.focus(function() {
			if(typeof(value) != 'undefined') {
				if( this.value == value ) {
					this.value = "";
				}
			} else {
				if( this.value == this.defaultValue ) {
					this.value = "";
				}
			}
        }).blur(function() {
			if(typeof(value) != 'undefined') {
				if( !this.value.length ) {
					this.value = value;
				}
			} else {
				if( !this.value.length ) {
					this.value = this.defaultValue;
				}
			}
        });
    };
	
	(function($) {
		jQuery.fn.passwordField = function(settings) {
			var passwordField = {
				init : function() {
					if($(this).attr('type') == 'password') {
						$(this).focus(function() {
							$(this).removeClass(passwordField.settings.className);
						});
						$(this).blur(function() {
							if( !$(this).val().length) {
								$(this).addClass(passwordField.settings.className);
							}
						});
						if(!$(this).val().length) {
							$(this).addClass(passwordField.settings.className);
						}
						passwordField.obj = $(this);
						passwordField.checkValue();
					}
				},
				checkValue : function() {
					if(passwordField.obj.val().length) {
						passwordField.obj.removeClass(passwordField.settings.className);
					}
					setTimeout(passwordField.checkValue, 10);
				}
			};
			var settings = $.extend({
				'className' : 'passwordField'
			},
			settings);
			passwordField.settings = settings;
			return passwordField.init.apply(this, arguments);
		};
	})( jQuery );
	
	$('#headerSearch').search();
	
	jQuery.validator.addMethod("defaultInvalid", function(value, element) {
		if($(element).attr('defaultValue') != "") {
			return !(element.value == $(element).attr('defaultValue'));
		}
		return !(element.value == element.defaultValue);
	});
	
	$('#headerSearchForm').validate({
	  rules: {
		q: {
			required: true,
			defaultInvalid: true
		}
	  },
	  messages: {
		q: {
			required : 'Required',
			defaultInvalid : 'Required'
		}
	  }
	});
	
	$(".popup-availability-notification-handle").colorbox({iframe:true, width:"500", height:"370"});

	$('.checkBox input.myCheckbox').prettyCheckboxes();
	
	$('.compareCheckbox').change(function() {
		var product_id = $(this).val();
		var compare = $(this).is(':checked');
		var obj = $(this);
		$.ajax({
			url: webRoot+'/ajax/compare_item.php',
			dataType:"json",
			data:{product_id : product_id, compare: compare, action: 'addRemove'},
			success: function(result) {
				if(result.status == 1) {
					$('.bottomSlide').show();
					$('.compareItemConatiner').html(result.html);
					$('.compareItemConatiner').show();
					$('#compareErrorContainer').html('');
					$('#compareItemMinMaxContainer').html('<a id="compareMinimize" href="#" class="minus" name="minus" title="minus">minus</a>');
				} else {
					if(result.error_code == 3) {
						$('#compareErrorContainer').html('');
						$('.bottomSlide').hide();
					} else if(result.error_code == 1 || result.error_code == 2) {
						$('#compareErrorContainer').html(result.error);
						$('.compareItemConatiner').html(result.html);
						if(result.error_code == 2) {
							obj.attr('checked', false);
							obj.next('label').removeClass('checked');						
						}
					}
				}
			}
		});
	});
	
	$('#compareMinimize').live('click', function(event) {
		$('.compareItemConatiner').hide();
		$(this).parent().html('<a id="compareMaximize" href="#" class="plus" name="plus" title="plus">plus</a>');
		$.ajax({
			url: webRoot+'/ajax/compare_item_min_max.php',
			data:{windowstatus : 'min'}
		});
		event.preventDefault();
	});
	
	$('#compareMaximize').live('click', function(event) {
		$('.compareItemConatiner').show();
		$(this).parent().html('<a id="compareMinimize" href="#" class="minus" name="minus" title="minus">minus</a>');
		$.ajax({
				url: webRoot+'/ajax/compare_item_min_max.php',
				data:{windowstatus : 'max'}
		});
		event.preventDefault();
	});
	
	$('#compareClose').live('click', function(event) {
		$('.bottomSlide').hide();
		$.ajax({url: webRoot+'/ajax/clear_compare_item.php'});
		event.preventDefault();
		$('.compareCheckbox').attr('checked', false);
		$('.compareCheckbox').next('label').removeClass('checked');
	});
	
	$('.customSelectBox').msDropDown({mainCSS:'dd2'}).data("dd");
	
	$('.mini4dLink').mouseover(function(){
		$('#mini4dActualLink').addClass('over');
	});
	
	$('.mini4dLink').mouseout(function(){
		$('#mini4dActualLink').removeClass('over');
	});
	
	$('.educationalLink').mouseover(function(){
		$('#educationalActualLink').addClass('over');
	});
	
	$('.educationalLink').mouseout(function(){
		$('#educationalActualLink').removeClass('over');
	});
	
	$('.ruleLink').mouseover(function(){
		$('#ruleActualLink').addClass('over');
	});
	
	$('.ruleLink').mouseout(function(){
		$('#ruleActualLink').removeClass('over');
	});
	
	$('.scheduleLink').mouseover(function(){
		$('#scheduleActualLink').addClass('over');
	});
	
	$('.scheduleLink').mouseout(function(){
		$('#scheduleActualLink').removeClass('over');
	});
	
	$('.newsLink').mouseover(function(){
		$('#newsActualLink').addClass('over');
	});
	
	$('.newsLink').mouseout(function(){
		$('#newsActualLink').removeClass('over');
	});
	
	$('.expressLink').mouseover(function(){
		$('#expressActualLink').addClass('over');
		$('.currentLink').show();
	});
	expressMenuHideFlag = false;
	$('.expressLink').mouseout(function(){
		expressMenuHideFlag = true;
		setTimeout(function(){
			if(expressMenuHideFlag) {
				$('.currentLink').hide();
				$('#expressActualLink').removeClass('over');
			}
      	},500);
	});
	$('.currentLink').mouseenter(function(){
		expressMenuHideFlag = false;
	});
	$('.currentLink').mouseover(function(){
		$('#expressActualLink').addClass('over');
		$('#sub-menu-first').addClass('subSelected');
	});
	$('.currentLink').mouseout(function(){
		$('#expressActualLink').removeClass('over');
		$('#sub-menu-first').removeClass('subSelected');
	});
	
	$(window).bind('resize', function(event) {
		correctHeader() ;
	});
	
	function correctHeader() {
		if($(window).width() <= 1040) {
			$('#mainHeader').css({width : '1027px'});
		} else {
			$('#mainHeader').css({width : '100%'});
		}
	}
	correctHeader();
	
	$(window).bind('resize', function(event) {
		$('.footerTop').css('width', $(window).width() + $(window).scrollLeft());
		$('.footerBtm').css('width', $(window).width() + $(window).scrollLeft());
	});
	$(window).bind('scroll', function(event) {
		$('.footerTop').css('width', $(window).width() + $(window).scrollLeft());
		$('.footerBtm').css('width', $(window).width() + $(window).scrollLeft());
	});
	
});

function loadCompareItem() {
	$.ajax({
		url: webRoot+'/ajax/compare_item.php',
		dataType:"json",
		data:{action: 'addRemove'},
		success: function(result) {
			if(result.status == 1) {
				$('.bottomSlide').show();
				$('.compareItemConatiner').html(result.html);
				$('#compareErrorContainer').html('');
			} else {
				if(result.error_code == 3) {
					$('#compareErrorContainer').html('');
					$('.bottomSlide').hide();
				}
			}
		}
	});
}

function formatPhone(text) {
	return text.replace(/(\d{3})(\d{3})(\d{4})/, "$1-$2-$3");
}

function ws(width,height) {
	document.write('<img src="'+webroot+'/images/common/spacer.gif" alt="" border="0" height="'+height+'" width="'+width+'" />');
}

/*
JSON.stringify = JSON.stringify || function (obj) {
    var t = typeof (obj);
    if (t != "object" || obj === null) {
      // simple data type
      if (t == "string") obj = '"'+obj+'"';
        return String(obj);
    } else {
    // recurse array or object
      var n, v, json = [], arr = (obj && obj.constructor == Array);
      for (n in obj) {
        v = obj[n]; t = typeof(v);
        if (t == "string") v = '"'+v+'"';
        else if (t == "object" && v !== null) v = JSON.stringify(v);
        json.push((arr ? "" : '"' + n + '":') + String(v));
      }
      return (arr ? "[" : "{") + String(json) + (arr ? "]" : "}");
    }
};

// implement JSON.parse de-serialization
JSON.parse = JSON.parse || function (str) {
    if (str === "") str = '""';
      eval("var p=" + str + ";");
      return p;
};
*/
function open_window(theURL,winName,features) { //v2.0

	window.open(theURL,winName,features);

}

function ws(width,height) {

	document.write('<img src="<?=Environment::webroot()?>/images/spacer.gif" alt="" border="0" height="'+height+'" width="'+width+'" />');

}