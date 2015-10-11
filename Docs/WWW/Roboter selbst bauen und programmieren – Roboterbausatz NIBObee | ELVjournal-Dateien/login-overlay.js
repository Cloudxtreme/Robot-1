function showLoginOverlay() {
	if (!jQuery("#login-overlay-bg").is(":visible")) {
		jQuery("#login-overlay-bg").fadeIn("fast");
		jQuery("#login-overlay").fadeIn("fast");
	}
}

function hideLoginOverlay() {
	if (jQuery("#login-overlay-bg").is(":visible")) {
		jQuery("#login-overlay-bg").fadeOut("fast");
		jQuery("#login-overlay").fadeOut("fast");
	}
}

jQuery(document).ready(function () {
	jQuery("#login-overlay").center();

	if (document.warenkorb) {
		jQuery("#login-overlay #login-overlay-form").attr("action", "/controller.aspx?cid=104&detail=21&detail2=1");
		jQuery("#login-overlay #logout-text").attr("onclick", "");
		jQuery("#login-overlay #logout-text").click(function () { document.location.href = "/login-registrieren.html?detail=13&returnwk=1"; });
		jQuery("#login-overlay #logout-image").attr("onclick", "");
		jQuery("#login-overlay #logout-image").click(function () { document.location.href = "/login-registrieren.html?detail=13&returnwk=1"; });
	} else {
		jQuery("#login-overlay #returnurl").val(this.href);
	}

	jQuery(".require-strong-auth").click(function () {
		jQuery("#login-overlay").center();

		if(this.href)
			jQuery("#login-overlay #returnurl").val(this.href);
		
		showLoginOverlay();
		return false;
	});

	jQuery(document).keypress(function (e) {
		if (e.keyCode == 27)
			hideLoginOverlay();
	});

	jQuery("#login-overlay-close-button").click(function () {
		hideLoginOverlay();
	});
});

jQuery(window).resize(function () {
	jQuery("#login-overlay").center();
});

jQuery.fn.center = function () {
	this.css("position", "absolute");
	this.css("top", Math.max(0, ((jQuery(window).height() - this.outerHeight()) / 2) + jQuery(window).scrollTop()) + "px");
	this.css("left", Math.max(0, ((jQuery(window).width() - this.outerWidth()) / 2) + jQuery(window).scrollLeft()) + "px");
	return this;
}