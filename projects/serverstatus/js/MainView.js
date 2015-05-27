(function () {
  "use strict";
  brite.registerView("MainView", {}, {
    
    create: function (data, config) {
      var view = this;
      var date = new Date();
      jQuery.extend(data, {
        now: date.getHours() + ":" + date.getMinutes() + ":" + date.getSeconds()
      });
      return jQuery("#tmpl-MainView").render(data);
    }
    
  });
}());