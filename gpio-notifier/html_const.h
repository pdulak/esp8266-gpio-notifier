const char HTML_header[] = R"=====(
<html>
  <head>
      <title>GPIO notifier</title>
  </head>
  <link rel="stylesheet" href="https://unpkg.com/purecss@2.0.5/build/pure-min.css" integrity="sha384-LTIDeidl25h2dPxrB2Ekgc9c7sEC3CWGM6HeFmuDNUjX76Ert4Z4IY714dhZHPLd" crossorigin="anonymous">
  <style type="text/css">
    .pure-u-22-24 { margin-left: 4.16% }
    .button-success { background: rgb(28, 184, 65); }
    .pin-status { padding: 5px 0; }
    .pin-status span.comment { display: block; color: lightgray; }
    .pin-status span.pin-state { font-size: 130%; font-weight: bold; }
    .pure-form legend { font-size: 200%; }
  </style>
<body>
  <div class="pure-g">
    <div class="pure-u-22-24">
)=====";

const char HTML_body[] = R"=====(
  <h3>Pin statuses:</h3>
  <input type="button" class="pure-button button-primary" value="Refresh" onclick="window.location.reload()">
)=====";

const char HTML_footer[] = R"=====(
    </div>
  </div>
  <script>
    function ShowForm() {
      document.getElementById('config-form').style.display='';
      document.getElementById('form-toggle').style.display='none';
    }

    function HideForm() {
      document.getElementById('config-form').style.display='none';
      document.getElementById('form-toggle').style.display='';
    }
  </script>
</body>
</html>
)=====";

const char HTML_form_header[] = R"=====(
  <input type="button" class="pure-button button-primary" value="Show configuration form" onclick="ShowForm()" id="form-toggle">
  <form action="/save" method="POST" class="pure-form pure-form-stacked" id="config-form" style="display: none">
    <h3>Pins handlers configuration:</h3>
)=====";

const char HTML_form_footer[] = R"=====(
    <input type="submit" class="pure-button pure-button-primary" value="Save handlers">
    <input type="button" class="pure-button" value="Discard and hide form" onclick="HideForm()" >
  </form>
)=====";

const char HTML_saved_button[] = R"=====(
  <a class="pure-button button-success" href="/" style="width: 100%; margin-top: 20px" id="success-button">Information saved - click here to return to the homepage</a>
  <script>
    setTimeout(function(){ document.getElementById('success-button').click() }, 2000);
  </script>
)=====";
