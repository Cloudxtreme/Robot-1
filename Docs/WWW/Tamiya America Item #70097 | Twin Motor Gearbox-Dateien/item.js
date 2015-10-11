$(document).ready(function($){
	$('input.myCheckbox').prettyCheckboxes();
	
	$('.tabHandle').click(function(event) {
		$(this).parent().find('li').removeClass('active');
		$(this).addClass('active');
		event.preventDefault();
		$('.tabStyle-2Contents').hide();
		$('#'+$(this).attr('id')+'-content').show();
	});
	$(".tamiyaGallery").colorbox({
		speed: 0,
		opacity : '.7',
		current: 'image {current} of {total}',
		rel:'tamiyaGallery',
		onOpen: function(){
			$('#cboxTopCenter').html('<div style="font-size: 22px; color:#FFFFFF; line-height:62px;">Image Gallery</div>');
			$("#colorbox").addClass("galleryColorbox");
			$('#cboxWrapper div:first').addClass('galleryTitle');
			var cboxWrapperwidth = $('#cboxWrapper').width()+80;
			$('#cboxWrapper').css({'width' : cboxWrapperwidth });
		},
		onResize: function(){
			var cboxWrapperwidth = $('#cboxContent').width() + 80;
			$('#cboxWrapper').css({'width' : cboxWrapperwidth });
			$('#colorbox').css({'top' : $('#colorbox').position().top - 50 });
			
			captionHeight = $('#cboxTitle').height();
			$('#cboxMiddleLeft').css({'height' : $('#cboxMiddleLeft').height() + captionHeight + 20 });
			$('#cboxContent').css({'height' : $('#cboxContent').height() + captionHeight  + 20 });
			$('#cboxMiddleRight').css({'height' : $('#cboxMiddleRight').height() + captionHeight  + 20 });
			$('#cboxCurrent').css({'top' : captionHeight + 20});
			$('#cboxTitle').css({'margin-top' : $('#cboxMiddleLeft').height()});
		},
		onClosed: function(){
			$('#cboxTopCenter').html('');
			$("#colorbox").removeClass("galleryColorbox");
			$('#cboxWrapper div:first').addClass('galleryTitle');
		}
	});
	$('#mycarousel').jcarousel();
	
	$(".popupEMailPageHandle").colorbox({iframe:true, width:"500", height:"370"});
	
	$("#qtyInput").spinner({max: 99, min: 1});
});