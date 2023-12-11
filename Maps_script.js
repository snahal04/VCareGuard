function doGet(e) {
  Logger.log(JSON.stringify(e));  // view parameters
  var result = 'Ok'; // assume success

  // Check if the 'value' parameter exists in the query parameters
  if (!e.parameter.value) {
    result = 'No Parameter named "value"';
  } else {
    var sheet_id = '1Ha6ap3USqMDXS67iOrA7RBHCeQkxMkzW3ywBVGEC0F0'; // Spreadsheet ID
    var sheet = SpreadsheetApp.openById(sheet_id).getActiveSheet(); // get Active sheet
    var newRow = sheet.getLastRow() + 1;
    var d = new Date();

    // Timestamp in column A
    // var timestamp = d.toLocaleTimeString();
    var options = { hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: true };
    var timestamp = d.toLocaleTimeString(undefined, options);

    // Value from the 'value' parameter
    var coordinates = stripQuotes(e.parameter.value);
    var mapLink = 'https://www.google.com/maps/search/?api=1&query=' + coordinates;
    var mapLinkText = 'View on Map';

    // Create a hyperlink formula
    var hyperlinkFormula = '=HYPERLINK("' + mapLink + '","' + mapLinkText + '")';

    // Write data to the specific columns
    sheet.getRange(newRow, 1).setValue(d);            // Timestamp in column A
    sheet.getRange(newRow, 2).setValue(timestamp);    // Timestamp in column B
    sheet.getRange(newRow, 3).setFormula(hyperlinkFormula);  // Hyperlink formula in column C

    var nearbyHospitals = findNearbyHospitals(coordinates);
    //////////////////////////////
    var hospitalNames = nearbyHospitals.map(function (hospital) {
      return hospital.name;
    }).join(',');

    // Encode the hospitalNames as a URL parameter
    var encodedHospitalNames = encodeURIComponent(hospitalNames);

    // Make an HTTPS request to Spreadsheet 1
    var spreadsheet1URL = 'https://script.google.com/macros/s/AKfycbwaYGkJE0O72P0cg2snxOVV1gHpTio0xFbkoKKMlBM6ABoA8vHDPJ38WA8d2GzduOtphg/exec?values=' + encodedHospitalNames;
    var response1 = UrlFetchApp.fetch(spreadsheet1URL);
    var responseText = response1.getContentText();

    var hospitalEmails = [];
    // result = responseText;
    // Write the 3 nearest hospital names and create direction links
    if (nearbyHospitals.length >= 3) {
      var responseLines = responseText.split('\n\n');
      for (var i = 0; i < 3; i++) {
        var hospitalData = responseLines[i].trim().split('\n');
        if (hospitalData.length >= 4) {
          var hospitalName = hospitalData[0].substring(13); // Extract Hospital Name
          var email = hospitalData[1].substring(7); // Extract Email
          hospitalEmails.push(email);
          var phoneNumber = hospitalData[2].substring(13); // Extract Phone Number
          var alternatePhoneNumber = hospitalData[3].substring(25); // Extract Alternate Phone Number

          var hospitalLocation = nearbyHospitals[i].location;
          var directionLink = 'https://www.google.com/maps/dir/' + coordinates + '/' + hospitalLocation.lat + ',' + hospitalLocation.lng;
          var directionLinkText = hospitalName;
          var directionHyperlinkFormula = '=HYPERLINK("' + directionLink + '","' + directionLinkText + '")';

          // Create comments for hospital contact details
          var hospitalComments = "Hospital Name: " + hospitalName + "\n";
          hospitalComments += "Email: " + email + "\n";
          hospitalComments += "Phone Number: " + phoneNumber + "\n";
          hospitalComments += "Alternate Phone No.: " + alternatePhoneNumber;

          // Insert the hyperlink formula in the cell and add the comment
          var cell = sheet.getRange(newRow, 4 + i);
          cell.setFormula(directionHyperlinkFormula);
          cell.setComment(hospitalComments);
        }
        // // 
        // var hospitalName = nearbyHospitals[i].name;
        // var hospitalLocation = nearbyHospitals[i].location;
        // var directionLink = 'https://www.google.com/maps/dir/' + coordinates + '/' + hospitalLocation.lat + ',' + hospitalLocation.lng;
        // var directionLinkText = hospitalName;
        // var directionHyperlinkFormula = '=HYPERLINK("' + directionLink + '","' + directionLinkText + '")';
        // sheet.getRange(newRow, 4 + i).setFormula(directionHyperlinkFormula);  // Hyperlink formula for Directions in columns D, E, F
      }
    }

  }

  // Return result of operation

  return ContentService.createTextOutput(hospitalEmails.join(','));
}

function stripQuotes(value) {
  return value.replace(/^["']|['"]$/g, "");
}

function findNearbyHospitals(coordinates) {
  // Your Google Places API request
  var apiKey = 'AIzaSyBgAC1y6dEPcJkNZJSjy_0Ec2KHEAC9NCs';
  var radius = 5000; // Define your preferred search radius in meters
  var url = 'https://maps.googleapis.com/maps/api/place/nearbysearch/json?location=' + coordinates + '&radius=' + radius + '&type=hospital&keyword=emergency&key=' + apiKey;

  var response = UrlFetchApp.fetch(url);
  var data = JSON.parse(response.getContentText());

  if (data.status === 'OK' && data.results.length > 0) {
    // data.results.sort(function(a, b) {
    //   return a.geometry.location.lat - b.geometry.location.lat +
    //          a.geometry.location.lng - b.geometry.location.lng;
    // });

    // Sort the hospitals by user_ratings_total in descending order
    data.results.sort(function (a, b) {
      return b.user_ratings_total - a.user_ratings_total;
    });

    var hospitals = data.results.slice(0, 3).map(function (hospital) {
      return {
        name: hospital.name,
        location: hospital.geometry.location
      };
    });
    return hospitals;
  } else {
    return 'Error: Unable to find nearby hospitals';
  }
}

