<!DOCTYPE html>
<html>
  <head>
    <title>Video for Teamspeak 3</title>
    <meta charset="utf-8">
    <meta name="google-site-verification" content="">
    <link rel="stylesheet" type="text/css" href="http://fonts.googleapis.com/css?family=Open+Sans|Roboto|Audiowide" />
    <link rel="stylesheet" type="text/css" href="css/index.css" />
  </head>
  <body>

    <div class="header">
      <div class="header-content">
        <h1 class="brand">
          <a href="./">TS3VIDEO</a>
        </h1>
        <div class="nav-links">
          <a href="./docs">Docs</a>
          <a href="#">Blog</a>
          <a href="#">Forum</a>
        </div>
        <div class="clearfix"></div>
      </div>
    </div>

    <div class="main-content">

      <?php if ($this->metaValue("type", "") === "frontpage"): ?>
        <?php $this->renderContent(); ?>
      <?php else: ?>
        <div class="content">
          <?php $this->renderContent(); ?>
        </div>
      <?php endif; ?>

    </div>

    <section class="footer">
      <div class="content">
        Copyright 2015 by Manuel Freiholz<br>
        Site created with <a href="http://humblecms.insanefactory.com" target="_blank">HumbleCMS</a>
      </div>
    </section>

  </body>
</html>