function doGet(e) {
  Logger.log(JSON.stringify(e)); // View parameters
  var result = 'Ok'; // Assume success

  // Check if the 'values' parameter exists in the query parameters
  if (!e.parameter.values) {
    result = 'No Parameter named "values"';
  } else {
    var searchValues = e.parameter.values.split(','); // Split the comma-separated values
    var sheet_id = 'REPLACE_WITH_YOUR_SPREADSHEET_ID'; // Spreadsheet ID
    var sheet = SpreadsheetApp.openById(sheet_id).getActiveSheet(); // Get Active sheet
    var data = sheet.getDataRange().getValues();
    var results = searchValuesInSheet(data, searchValues);

    if (results.length > 0) {
      // Construct a response with data for all matched hospitals
      result = '';
      for (var i = 0; i < results.length; i++) {
        result += 'Hospital Name: ' + results[i][0] + '\n' +
                  'Email: ' + results[i][1] + '\n' +
                  'Phone Number: ' + results[i][2] + '\n' +
                  'Alternate Phone Number: ' + results[i][3] + '\n\n';
      }
    } else {
      result = 'Hospitals not found in the sheet.';
    }
  }

  // Return result of the operation
  return ContentService.createTextOutput(result);
}

function searchValuesInSheet(data, searchValues) {
  var results = [];

  for (var j = 0; j < searchValues.length; j++) {
    var searchValue = searchValues[j];
    if (searchValue == "00") break;
    for (var i = 0; i < data.length; i++) {
      if (data[i][0] === searchValue) {
        // Hospital name found in column A (index 0)
        results.push([data[i][0], data[i][1], data[i][2], data[i][3]]);
        break; // No need to continue searching for this value
      }
    }
  }

  return results;
}
