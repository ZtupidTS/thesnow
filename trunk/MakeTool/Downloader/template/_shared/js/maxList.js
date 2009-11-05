////////////////////////////////////////////////////////////////////////
// maxList v1.0.0
// 2007-08-29 11:23:07
// SiC
////////////////////////////////////////////////////////////////////////
var maxList = {};

//----------------------------------------------------------
// Create maxList Data Object
//----------------------------------------------------------
maxList.create = function(id){

	return {

		"objID" : id, // The ID of Javascript object
		"id" : id, // The ID of <DIV> Element - seldom need to change
		"wrapperID" : id + "_wrapper", // The ID of wrapper Element - seldom need to change

		// Styles - CSS Classes
		"style" : {
			"list": 'maxList', // <DIV> wrapper tag
			"itemWrapper": 'maxList-itemWrapper', // item wrapper - <DIV>
			"normal": 'maxList-normal',			// Normal Row
			"hover": 'maxList-hover',	// Hovered Row
			"selected": 'maxList-selected'		// Selected Row
		},

		// Data - All Required
		"data" : [],	// The Data Array

		// Selected Item index
		"selectedIndex" : -1,

		// Icon Click Action
		"iconAction" : "none", // "delete" or "checkbox", default is "none", no action

		// Language array with default text
		"lang": {
			"delete_confirm": "Are you sure to delete this item?"
		},

		"controlUI": {
			"delete": '<img src="images/btn_delete.png" />',
			"icon": '<img src="images/item_icon.png" />',
			"checkbox": '<img src="images/btn_check.png" />',
			"checkbox_checked": '<img src="images/btn_check_checked.png" />'
		},

		// Content Handles - must given
		"content": {
			"buildItem": null, // build item html
			"getIcon": null // get item icon
		},

		// Event Handles
		"event": {
			// Called when init a table
			"beforeInit": function(){ return true;},
			"afterInit": function(){},
			// Called when destroy a table
			"beforeDestroy": function(){ return true;},
			"afterDestroy": function(){},
			// Called when edit a row
			"beforeSelect": function(){ return true;},
			"afterSelect": function(){},
			// Called when cancel the edit of a row
			"beforeLostFocus": function(){ return true;},
			"afterLostFocus": function(){},
			// Called when the a row is deleted
			"beforeDelete": function(deletedItem){ return true;},
			"afterDelete": function(deletedItem){}
		},

		// Private Variables - you can access these variables in Event Handle Functions
		"inAction" : "" // "init", "select"

	};

}


//----------------------------------------------------------
// Initialize a table and paint to visual page
//----------------------------------------------------------
maxList.init = function(objList){

	objList.inAction = "init";
	if(!objList.event.beforeInit()) return false;

	maxList.ui.attachList(objList);

	objList.event.afterInit();

	objList.inAction = "";

}


//----------------------------------------------------------
// Destroy a table control
//----------------------------------------------------------
maxList.destroy = function(objList){

	objList.inAction = "destroy";

	if(!objList.event.beforeDestroy()) return false;

	maxList.ui.detachTable(objList);

	objList.event.afterDestroy();

	delete objList;

}


//----------------------------------------------------------
// Update the whole list
//----------------------------------------------------------
maxList.update = function(objList){

	maxList.ui.attachList(objList);

}


//**********************************************************
// UI Functions
//**********************************************************
maxList.ui = {};

//----------------------------------------------------------
// Build HTML for maxList Object
//----------------------------------------------------------
maxList.ui.attachList = function(objList){

	if(!(objList.data instanceof Array)){
		alert("maxList.ui.attachList : " + objList.id + ".data is not an Array\n" + $toJSON(objList.data));
		return;
	}

	var obj = $id(objList.wrapperID);

	if(obj){
		obj.innerHTML = maxList.ui.buildList(objList);
	}else{
		alert("maxList.ui.attachList : " + objList.wrapperID + " not Found");
	}

}


//----------------------------------------------------------
// Remove HTML for maxList Object
//----------------------------------------------------------
maxList.ui.detachList = function(objList){

	var obj = $id(objList.wrapperID);
	if(obj){
		obj.innerHTML = "";
	}

}


//----------------------------------------------------------
// Build HTML for maxList Object
//----------------------------------------------------------
maxList.ui.buildList = function(objList){

	var output = '<div id="' + objList.id + '" class="' + objList.style["list"] + '">';

	for(var i=0;i<objList.data.length;i++){

		output += maxList.ui.buildItem(objList, i);

	}

	output += '</div>'

	return output;

}


//----------------------------------------------------------
// Build List Row HTML
//----------------------------------------------------------
maxList.ui.buildItem = function(objList, rowIndex){

	var item = objList.data[rowIndex];
	var output = '<div class="' + objList.style["itemWrapper"] + '">'
				+ '<div id="' + objList.id + '_item_' + rowIndex + '"'
				+ ' class="' + objList.style["normal"] + '"'
				+ ' onclick="maxList.action.selectItem(' + objList.objID + ', ' + rowIndex + ')"'
				+ ' onmousemove="maxList.ui.hoverItem(' + objList.objID + ', ' + rowIndex + ', this, true)"'
				+ ' onmouseout="maxList.ui.hoverItem(' + objList.objID + ', ' + rowIndex + ', this, false)"'
				+ '>';

	output += '<img id="'+ objList.id +"_item_"+ rowIndex + '_img" src="' + objList.controlUI["itemIcon"] + '"'
			+ ' onclick="maxList.action.clickIcon(' + objList.objID + ', ' + rowIndex + ');" /> ';

	output += objList.content.buildItem(item);

	output += '</div></div>';

	return output;

}


//----------------------------------------------------------
// Update Item HTML
//----------------------------------------------------------
maxList.ui.updateItem = function(objList, rowIndex){

	var obj = $id(objList.id + '_item_' + rowIndex);

	if(!obj) return;

	obj.innerHTML = maxList.ui.buildItem(objList, rowIndex);

}


//----------------------------------------------------------
// Hover Effect
//----------------------------------------------------------
maxList.ui.hoverItem = function(objList, rowIndex, objDIV, isHover){


	// icon button effect
	var objIMG = $id(objList.id +"_item_"+ rowIndex + "_img");
	if(isHover){
		objIMG.src = objList.iconAction == "delete" ? objList.controlUI["deleteIcon"] : objList.controlUI["checkIcon"];
	}else{
		objIMG.src = objList.controlUI["itemIcon"];
	}

	// css effect
	if(objList.selectedIndex == rowIndex) return;

	if(isHover){
		objDIV.className = objList.style["hover"];
	}else{
		objDIV.className = objList.style["normal"];
	}


}


//**********************************************************
// Action Functions
//**********************************************************
maxList.action = {};

//----------------------------------------------------------
// On Item selected
//----------------------------------------------------------
maxList.action.selectItem = function (objList, rowIndex){

	objList.inAction = "select";

	if(!objList.event.beforeSelect()) return false;

	var obj = $id(objList.id + '_item_' + objList.selectedIndex);
	if(obj) obj.className = objList.style["normal"];

	obj = $id(objList.id + '_item_' + rowIndex);
	if(obj) obj.className = objList.style["selected"];

	objList.selectedIndex = rowIndex;

	objList.event.afterSelect();

}


//----------------------------------------------------------
// On Icon Clicked
//----------------------------------------------------------
maxList.action.clickIcon = function (objList, rowIndex){

	if(objList.iconAction == "delete"){
		maxList.action.deleteItem(objList, rowIndex);
	}

}


//----------------------------------------------------------
// Delete an Item
//----------------------------------------------------------
maxList.action.deleteItem = function(objList, rowIndex){

	window.event.cancelBubble = true;
	window.event.returnValue = false;

	if(!objList.event.beforeDelete(objList.data[rowIndex])) return;

	if(!confirm(objList.lang["delete_confirm"])) return;

	// Update data
	var deletedItem = objList.data[rowIndex];
	objList.data.splice(rowIndex, 1);
	objList.selectedIndex = 0;

	// update
	maxList.update(objList);

	// Fire Custom Event
	objList.event.afterDelete(deletedItem);


}
