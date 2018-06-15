function getCookie(cookie_n) {
    			var name = cookie_n + "=";
    			var array = document.cookie.split(';');
		    	for(var i = 0; i < array.length; i++) {
		       				var c = array[i];
		        			while (c.charAt(0) == ' ') {
				            c = c.substring(1);
				        }
				        if (c.indexOf(name) == 0) {
				            return c.substring(name.length, c.length);
				        }
				    }
				    return "";
				}
				$(document).ready(function() {
					var shouldShow = 0;
					if (getCookie("shouldShow") == "") {
						shouldShow = 0;
					}
					else {
						shouldShow = getCookie("shouldShow");
					}
					if (shouldShow == 0) {
						$(".location_table_body").toggleClass("hidden");
					}
					$("#location").click(function() {
						shouldShow++;
	  					shouldShow = shouldShow % 2;
					  	$(".location_table_body").slideToggle("hidden");
					  	document.cookie = "shouldShow" + "=" + shouldShow;
					});
				});	
