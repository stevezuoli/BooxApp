$(function() {
  var images = ["images0.jpeg", "images1.jpeg", "images2.jpeg",
                "images3.jpeg", "images4.jpeg", "images5.jpeg"];
  var selected = 0;

  function updateRegion(x, y, width, height, method) {
    console.log("Updating region: " + x + ", " + y + ", " + width + ", " +
                height);
    if (window.screenUpdater) {
      window.screenUpdater.updateRegion(x, y, width, height, method);
    } else {
      console.log("Ignoring update request");
    }
  }

  function updateElement(ele) {
    updateRegion(ele.offset().left, ele.offset().top, ele.outerWidth(),
                 ele.outerHeight(), "GC");
  }

  function init() {
    for (var i = 0; i < images.length; ++i) {
      var html = "<img id=\"image-" + i +"\" src=\"" + images[i] +"\" />";
      $(html).appendTo("#container");
      updateElement($("#image-" + i));
    }
    if (window.testObj) {
      window.testObj.testMap({"a" : 1, "b" : 2});
      window.testObj.testVec([1, 2, 3]);
      var list = window.testObj.testGetList();
      var value = 0;
      for (var i = 0; i < list.length; ++i)
      {
        value = value + list[i];
      }
      alert(value);
    }
    if (window.onyxDB) {
      window.onyxDB.open("test");
      window.onyxDB.insert("book", {"title": "C Programming", "id": 1});
      var books = window.onyxDB.select("book", {"id": 1});
      alert(books[0]["title"]);
    }
  }

  function renderBorders() {
    for (var i = 0; i < images.length; ++i) {
      var img = $("#image-" + i);
      if (i == selected) {
        if (!img.hasClass("selected")) {
          img.addClass("selected");
          updateElement(img);
        }
      } else {
        if (img.hasClass("selected")) {
          img.removeClass("selected");
          updateElement(img);
        }
      }
    }
  }

  function bindEvents() {
    $(document).keydown(function(event) {
      var right = 39;
      var left = 37;
      if (event.keyCode == right) {
        selected = Math.min(images.length-1, selected+1);
      } else if (event.keyCode == left) {
        selected = Math.max(0, selected-1);
      }
      renderBorders();
    });
  }

  init();
  renderBorders();
  bindEvents();
});
