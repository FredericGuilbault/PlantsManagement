let currentPage = 1;
jQuery( document ).ready(function() {
    // every time we load a full webpage, it's empty. we need to query the data of at least one page. 
    loadAllDataOfPage(currentPage)

    jQuery('.nextPrev.prev').on( "click",function(){
        currentPage = currentPage-1;
        if( currentPage < 0 ){ currentPage = 0 } 
        loadAllDataOfPage(currentPage)
    })

    jQuery('.nextPrev.next').on( "click",function(){
        currentPage = currentPage+1;
        if( currentPage > 10 ){ currentPage = 10 } 
        loadAllDataOfPage(currentPage)
    })  

    jQuery('.setAuto').on( "click",function(){
        check = jQuery('#applyToAllLightsph').prop( "checked")
        if( ! check || currentPage < 2 ){
        jQuery.getJSON('/set?page='+currentPage+'&status=auto', function(jd) {
            refreshData(jd,currentPage)
        });
    }
    })

    jQuery('.setOn').on( "click",function(){ 
        check = jQuery('#applyToAllLightsph').prop( "checked")
        if( ! check || currentPage < 2 ){
        jQuery.getJSON('/set?page='+currentPage+'&status=on', function(jd) {
            refreshData(jd,currentPage)
        });
    }
    })

    jQuery('.setOff').on( "click",function(){ 
        check = jQuery('#applyToAllLightsph').prop( "checked")
        if( ! check || currentPage < 2 ){
        jQuery.getJSON('/set?page='+currentPage+'&status=off', function(jd) {
            refreshData(jd,currentPage)
        });
    }
    })

    jQuery('.stoptimeph').focusout(function(){ 
        console.log( "foccus out ")
        let stop = jQuery(".stoptimeph").val()
        jQuery.getJSON('/set?page='+currentPage+'&stopTime='+stop, function(jd) {
            refreshData(jd,currentPage)
        });
    })

    jQuery('.starttimeph').focusout(function(){ 
        let start = jQuery(".starttimeph").val()
        console.log( "foccus out ")
       
        jQuery.getJSON('/set?page='+currentPage+'&startTime='+start, function(jd) {
            refreshData(jd,currentPage)
        });
    })

    jQuery('#applyToAllLightsph').on( "click",function(){ 
        let check = jQuery('#applyToAllLightsph').prop( "checked")
        if (check == true){
            check = 1
        } else{
            check = 0 
        }
            jQuery.getJSON('/set?page='+currentPage+'&applyToAllLights='+check, function(jd) {
                refreshData(jd,currentPage)
        });
    })

});

function loadAllDataOfPage(pageNumber){
    jQuery.getJSON('/get?page='+pageNumber, function(jd) {

        if(pageNumber == 0 ){
            jQuery('.nextPrev.prev').hide() 
            jQuery('.nextPrev.next').show()
        }else if (pageNumber >= 10 ){
            jQuery('.nextPrev.prev').show() 
            jQuery('.nextPrev.next').hide() 
        } else {
            jQuery('.nextPrev.prev').show() 
            jQuery('.nextPrev.next').show()     
        } 
        refreshData(jd,pageNumber)
       
},
function(data){
    console.log(data)
});

}

function refreshData(jd,pageNumber){

    if(pageNumber == 0 ){
        jQuery('.applyToAllLightsdiv').show(); 
        if(jd.applyToAllLights == 1 ){
            jQuery('#applyToAllLightsph').prop( "checked", true)
        }else{
            jQuery('#applyToAllLightsph').prop( "checked", false)
        }
       
    }else{
        jQuery('.applyToAllLightsdiv').hide(); 
    }
    
    if( jd.applyToAllLights == 1 && pageNumber > 1 ){
        jQuery('.starttimeph').prop('disabled', true);
        jQuery('.stoptimeph').prop('disabled', true); 
        jQuery('.radioBtn input').prop( "disabled", true ); 
    }else{
        jQuery('.starttimeph').prop('disabled', false);
        jQuery('.stoptimeph').prop('disabled', false); 
        jQuery('.radioBtn input').prop( "disabled", false ); 

    }

    console.log(jd)
    //  { label: "Pump", pinNumber: "4", status: "auto", overridable: "0", startTime: "15:00:00", stopTime: "15:00:30", applyToAllLights: "1" }
    jQuery('#labelPh').text( jd.label );
    jQuery('.radioBtn input[value~='+jd.status+']').prop( "checked", true ); 
    jQuery('.starttimeph').prop( "value", jd.startTime );
    jQuery('.stoptimeph').prop("value",jd.stopTime );
    
    printDelay();

}

function printDelay(){
    let startTime = jQuery('.starttimeph').val();
    let stopTime  = jQuery('.stoptimeph').val();
    var startDate = new Date("1/1/1900 " + startTime);
    var endDate = new Date("1/1/1900 " + stopTime );
    var difftime=endDate - startDate; //diff in milliseconds
    jQuery('.timevalue').text( msToTime(difftime));    
}

function msToTime(s) {
    var ms = s % 1000;
    s = (s - ms) / 1000;
    var secs = s % 60;
    s = (s - secs) / 60;
    var mins = s % 60;
    var hrs = (s - mins) / 60;
    return hrs + ':' + mins + ':' + secs ;
  }