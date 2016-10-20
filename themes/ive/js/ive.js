(function($, Drupal, drupalSettings){
	$(document).ready(function(){
		/*alert('Hello world!');*/
		$('a[href^="http"]').attr('target', '_blank');
		$('a[href^="http"]').addClass('External_link');
		$(".block h2").click(function () {
/*alert('Hello world!');*/
    $content = $(this).siblings('.content');
    //open up the content needed - toggle the slide- if visible, slide up, if not slidedown.
    $content.slideToggle(500, function () {
        //execute this after slideToggle is done
    });

});
	});
})(jQuery, Drupal, drupalSettings);



