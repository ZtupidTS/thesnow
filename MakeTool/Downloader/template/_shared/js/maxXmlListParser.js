var maxXmlListParser={};maxXmlListParser.error=null;maxXmlListParser.xmlDOM=null;maxXmlListParser.getXMLDOM=function(C){var A;try{if(window.ActiveXObject){A=external.max_activex("","Microsoft.XMLDOM");A.async=false;if(C.namespace){A.setProperty("SelectionNamespaces",C.namespace)}}else{var B=C.namespace?C.namespace:"";A=document.implementation.createDocument(B,"",null)}}catch(D){maxXmlListParser.error="Create XMLDOM Failed :: ("+D.number+") "+D.description;return null}return A};maxXmlListParser.parseFile=function(A,B){return maxXmlListParser.parse(A,B,true)};maxXmlListParser.parse=function(A,C,B){if(maxXmlListParser.load(A,C,B)){return maxXmlListParser.parseItems(maxXmlListParser.xmlDOM,C)}else{return null}};maxXmlListParser.load=function(A,C,B){if(!B){B=false}if(!maxXmlListParser.xmlDOM){maxXmlListParser.xmlDOM=maxXmlListParser.getXMLDOM(C)}if(!maxXmlListParser.xmlDOM){return false}isOK=true;if(B){maxXmlListParser.xmlDOM.async=false;maxXmlListParser.xmlDOM.load(A)}else{if(window.ActiveXObject){maxXmlListParser.xmlDOM.loadXML(A)}else{maxXmlListParser.xmlDOM=(new DOMParser()).parseFromString(A,"text/xml")}}if(window.ActiveXObject){if(maxXmlListParser.xmlDOM.parseError.errorCode!=0){isOK=false}}else{if(maxXmlListParser.xmlDOM.documentElement.tagName=="parsererror"){isOK=false}}if(isOK){return true}else{maxXmlListParser.error="Parse Error :: ("+maxXmlListParser.xmlDOM.parseError.errorCode+") "+maxXmlListParser.xmlDOM.parseError.reason;return false}};maxXmlListParser.selectNodes=function(B,D,H){if(window.ActiveXObject){if(H){return B.selectSingleNode(D)}else{return B.selectNodes(D)}}else{var F=new XPathEvaluator();var C=F.createNSResolver(B.ownerDocument==null?B.documentElement:B.ownerDocument.documentElement);var E=F.evaluate(D,B,C,0,null);var A=[];var G;while(G=E.iterateNext()){if(H){return G}else{A.push(G)}}if(A.length==0){return null}else{return A}}};maxXmlListParser.parseItems=function(B,F){var A=[];if(!maxXmlListParser.selectNodes(B.documentElement,F.validate,true)){return null}var C=maxXmlListParser.selectNodes(B.documentElement,F.items);for(var D=0;D<C.length;D++){var E=maxXmlListParser.parseSingleItem(C[D],F);if(E!={}){A.push(E)}}return A};maxXmlListParser.parseSingleItem=function(E,D){var A=D.itemTemplate?$clone(D.itemTemplate):{};for(var B in D.attributes){var C=maxXmlListParser.selectNodes(E,D.attributes[B],true);if(C){A[B]=C.nodeValue}}return A};maxXmlListParser.updateItem=function(C){if(maxXmlListParser.load(C.xml,{},C.isFile)){maxXmlListParser.xmlDOM.setProperty("SelectionLanguage","XPath");var A=maxXmlListParser.xmlDOM.selectSingleNode(C.xpath);if(A){A.nodeValue=C.value}if(C.save&&C.isFile){try{maxXmlListParser.xmlDOM.save(C.xml)}catch(B){alert(B.description)}}return true}else{alert(maxXmlListParser.error);return false}};