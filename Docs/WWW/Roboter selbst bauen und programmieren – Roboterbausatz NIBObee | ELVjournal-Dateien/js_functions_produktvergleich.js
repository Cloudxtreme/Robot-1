
var ProductCompareList = ',';
var UniqueID = 0;

function produktvergleich(art_id, add_text, add_link, open_text, open_link, max_link)
{

	var instance = this;

	this.art_id = art_id;
	this.divID = 'produktvergleich_' + this.art_id + '_' + (UniqueID++);
	this.div = null;
	this.add_text = add_text;
	this.add_link = add_link;
	this.open_text = open_text;
	this.open_link = open_link;
	this.max_link = max_link;

	this.showOpenlink = function() {

		var childNodes_length = instance.div.childNodes.length;
		for (var i=0; i<childNodes_length; i++)
			instance.div.removeChild(instance.div.childNodes[0]);

		var objLINK = document.createElement('a');
		objLINK.setAttribute('href', instance.open_link);
		//objLINK.setAttribute('target', '_blank');
		objLINK.innerHTML = instance.open_text;
		instance.div.appendChild(objLINK);
	};

	this.addProduct = function() {
		if (ProductCompareList.indexOf(',' + instance.art_id + ',') < 0)
		{
			var url = instance.add_link + '?artid=' + instance.art_id;

			new Ajax.Request(url, {
				method: 'get',
				onSuccess: function(transport) {
					if (transport.responseText == 'ok')
					{
						ProductCompareList += instance.art_id + ',';
						instance.showOpenlink();
					}
					else
					{
						if (transport.responseText == 'fehler: maximale Anzahl von Produkten zum Vergleich erreicht')
						{
							window.scrollTo(0, 0);
							document.getElementById('warenkorb_layer').style.display='';
							document.getElementById('iframe_warenkorb').src=instance.max_link;
						}
						else
						{
							window.alert('' + transport.responseText);
						}
					};
				},
				onFailure: function(transport) {
					alert('Es ist ein Fehler aufgetreten und damit konnte das Produkt nicht zum Produktvergleich hinzugefuegt werden.');
				}
			});
		};
		//instance.showOpenlink();
	};

	this.create = function() {
		document.write('<div id="' + this.divID + '" class="produktvergleich"></div>');
		this.div = document.getElementById(this.divID);

		if (ProductCompareList.indexOf(',' + instance.art_id + ',') < 0)
		{
			// AddFunction
			var objINPUT = document.createElement('input');
			objINPUT.setAttribute('id', this.divID + '_checkbox');
			objINPUT.setAttribute('name', this.divID + '_checkbox');
			objINPUT.setAttribute('type', 'checkbox');
			objINPUT.onclick = this.addProduct;
			this.div.appendChild(objINPUT);
			var objLABEL = document.createElement('label');
			//objLABEL.setAttribute('for', this.divID + '_checkbox');
			objLABEL.innerHTML = this.add_text;
			objLABEL.onclick = this.addProduct;
			this.div.appendChild(objLABEL);
		}
		else
		{
			this.showOpenlink();
		}
	};
	this.create();

}

function setStyle( object, styleText ) { if( object.style.setAttribute ) { object.style.setAttribute("cssText", styleText ); } else { object.setAttribute("style", styleText ); } }

function fixedColumn(tableID, Column) {

  var objTable = document.getElementById(tableID);

  // Headercells
  var objThead = objTable.getElementsByTagName('thead')[0];
  if (objThead) {
    var objTRs = objThead.getElementsByTagName("tr");
    for (var j = 0; j < objTRs.length; j++) {
      var objTR = objTRs[j];
      var objTD = objTR.getElementsByTagName("th")[Column];

      if (objTD) {
        // In DIV klonen
        var tmpDIV = document.createElement('div');
        var tmpStyles = "background-color: #F1F2F4; height: " + (objTD.offsetHeight-6) + "px; width: " + (objTD.offsetWidth-6) + "px; margin-top: -4px; margin-left: -4px; padding: 3px; position: absolute; border: solid 1px #CFCFCF; text-align: left;";
        if (objTD.style.borderTop != '') tmpStyles += "border-top: " + objTD.style.borderTop;
        if (objTD.style.borderBottom != '') tmpStyles += "border-bottom: " + objTD.style.borderBottom;
        //tmpDIV.setAttribute("style", tmpStyles);
        setStyle(tmpDIV, tmpStyles);
        tmpDIV.innerHTML = objTD.innerHTML;
        objTD.insertBefore(tmpDIV, objTD.firstChild);
      }
    }
  }

  var objTbody = objTable.getElementsByTagName('tbody')[0];
  if (objTbody) {
    var objTRs = objTbody.getElementsByTagName("tr");
    for (var j = 0; j < objTRs.length; j++) {
      var objTR = objTRs[j];
      var objTD = objTR.getElementsByTagName("th")[Column];

      if (objTD) {
        // In DIV klonen
        var tmpDIV = document.createElement('div');
        var tmpStyles = "background-color: white; height: " + (objTD.offsetHeight-6) + "px; width: " + (objTD.offsetWidth-6) + "px; margin-top: -4px; margin-left: -4px; padding: 3px; position: absolute; border: solid 1px #CFCFCF; text-align: left;";
        if (objTD.style.borderTop != '') tmpStyles += "border-top: " + objTD.style.borderTop;
        if (objTD.style.borderBottom != '') tmpStyles += "border-bottom: " + objTD.style.borderBottom;
        //tmpDIV.setAttribute("style", tmpStyles);
        setStyle(tmpDIV, tmpStyles);
        tmpDIV.innerHTML = objTD.innerHTML;
        objTD.insertBefore(tmpDIV, objTD.firstChild);
      }
    }
  }

}

