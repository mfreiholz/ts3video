(function () {
  "use strict";
  brite.registerView("MainView", {}, {
    
    create: function (data, config) {
      var view = this;
      return jQuery("#tmpl-MainView").render(data);
    }
    
  });
}());