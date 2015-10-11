/** 
  * async-pixel-loader
  * 
  * @author JSP / ydworld.com
  * @build 20120620-01
  *
  *    @usage 1.) save .js file to your CDN
  *    @usage 2.) invoke ydPixel() with:
  *    @usage var a = new Array();
  *    @usage a['productid'] = '{productid}';
  *    @usage ydPixel(YOURPIXELID, yd_data);
  *
  */ 
function ydPixel(yd_pixelid, yd_pixeldata)
{
	var yd_protocol, yd_secure, yd_pixelargs; // check base protocol, set calls accordingly
	
	if (document.location.protocol == 'https:') { yd_protocol = 'https://'; yd_secure = '&secure=true'; }
	else { yd_protocol = 'http://'; yd_secure = ''; }
	
    for (i in yd_pixeldata) // prepare data for pixel
    {
        if (yd_pixeldata[i]) 
        {
			if (typeof yd_pixeldata[i] !== 'function') {
				if (yd_pixelargs != undefined) { yd_pixelargs += '&' + i + '=' + yd_pixeldata[i] } 
				else { yd_pixelargs = '&' + i + '=' + yd_pixeldata[i] } 
			}
        }
    }
    
    yd_pixelargs = typeof(yd_pixelargs) != 'undefined' ? yd_pixelargs : '';

    document.write('<iframe src="' + yd_protocol + 'd.254a.com/pixel?id=' + yd_pixelid + '&t=3' + yd_secure + yd_pixelargs + '" width="0" height="0" frameborder="0"></iframe>'); // write iframe for pixel
    
}