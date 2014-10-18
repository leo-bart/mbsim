<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <!-- The html header -->
  <xsl:template match="/valgrindoutput">
    <html>
      <head>
        <link rel="stylesheet" href="http://maxcdn.bootstrapcdn.com/bootstrap/3.2.0/css/bootstrap.min.css"/>
        <link rel="stylesheet" href="http://cdn.datatables.net/1.10.2/css/jquery.dataTables.css"/>
        <script type="text/javascript" src="http://code.jquery.com/jquery-2.1.1.min.js"> </script>
        <script type="text/javascript" src="http://maxcdn.bootstrapcdn.com/bootstrap/3.2.0/js/bootstrap.min.js"> </script>
        <script type="text/javascript" src="http://cdn.datatables.net/1.10.2/js/jquery.dataTables.min.js"> </script>
        <script type="text/javascript">
          $(document).ready(function() {
            // initialize the error table
            $("#ErrorTable").dataTable({
              'lengthMenu': [ [10, 20, 50, 100, 200, 500, -1], [10, 20, 50, 100, 200, 500, 'All'] ],
              'pageLength': 50,
              'aaSorting': [],
              'stateSave': true
            });
            // initialize the stack location table
            $(".ErrorStackTable").dataTable({
              "language": {
                "lengthMenu": "Show _MENU_ stack locations in order",
              },
              'lengthMenu': [ [1, 2, 3, 5, 10, 20, -1], [1, 2, 3, 5, 10, 20, 'All'] ],
              'pageLength': 3,
              'bSort' : false,
              'stateSave': true
            });
          });
        </script>
      </head>
      <body style="margin:1em">
        <h1>Valgrind Report: <xsl:value-of select="tool/text()"/></h1>
        <h2>Call Information</h2>
        <dl class="dl-horizontal">
          <dt>Analysed Program</dt>
          <dd><code>
            <!-- the program being run including its arguments -->
            <xsl:value-of select="args/argv/exe/text()"/>
            <xsl:apply-templates select="args/argv/arg"/>
          </code></dd>
          <dt>Valgrind</dt>
          <dd><code>
            <!-- the program being run including its arguments -->
            <xsl:value-of select="args/vargv/exe/text()"/>
            <xsl:apply-templates select="args/vargv/arg"/>
          </code></dd>
        </dl>
        <h2>Errors</h2>
        <!-- a table of all errors: 1 column: error type; 2 column error details which is itself a table -->
        <table id="ErrorTable" class="table table-bordered">
          <thead><tr><th>Error Type</th><th>Error Details</th></tr></thead>
          <tbody>
            <xsl:apply-templates select="error"/>
          </tbody>
        </table>
      </body>
    </html>
  </xsl:template>

  <!-- argument of the program being run, seperated by spaces -->
  <xsl:template match="arg">
    <xsl:text> </xsl:text><xsl:value-of select="text()"/>
  </xsl:template>

  <!-- one row of the error table -->
  <xsl:template match="error">
    <!-- Leak_StillReachable is not an error -> don't show it -->
    <xsl:if test="kind/text()!='Leak_StillReachable'">
      <tr>
        <!-- error type -->
        <td class="danger">
          <h3><xsl:value-of select="kind/text()"/></h3>
          <!-- suppression raw text, if existing -->
          <xsl:if test="suppression/rawtext">
            <div class="btn-group btn-group-sm">
              <button type="button" class="btn btn-info dropdown-toggle" data-toggle="dropdown">
                Show suppression <span class="caret"></span>
              </button>
              <pre class="dropdown-menu" role="menu"><xsl:value-of select="suppression/rawtext/text()"/></pre>
            </div>
          </xsl:if>
        </td>
        <!-- error details -->
        <td>
          <!-- location stack of the valgrind "what" message -->
          <h4 class="text-danger"><xsl:value-of select="what/text()"/></h4>
          <xsl:apply-templates select="what/following-sibling::stack[position()=1]"/>
          <!-- location stack of the valgrind "auxwhat" message, if existing -->
          <xsl:if test="auxwhat">
            <h4 class="text-danger"><xsl:value-of select="auxwhat/text()"/></h4>
            <xsl:apply-templates select="auxwhat/following-sibling::stack[position()=1]"/>
          </xsl:if>
          <!-- location stack of the valgrind "xwhat" message, if existing -->
          <xsl:if test="xwhat">
            <h4 class="text-danger"><xsl:value-of select="xwhat/text/text()"/></h4>
            <xsl:apply-templates select="xwhat/following-sibling::stack[position()=1]"/>
          </xsl:if>
        </td>
      </tr>
    </xsl:if>
  </xsl:template>

  <!-- a table for the location stack -->
  <xsl:template match="stack">
    <table class="ErrorStackTable table table-striped table-hover table-bordered table-condensed">
      <thead><tr><th>File:Line</th><th>Function Name</th><th>Library</th></tr></thead>
      <tbody>
        <!-- each stack entry is a table row -->
        <xsl:apply-templates select="frame"/>
      </tbody>
    </table>
  </xsl:template>

  <!-- one row of the location stack table -->
  <xsl:template match="frame">
    <!-- skip rows without a function name: these are very OS internal -->
    <xsl:if test="fn">
      <tr>
        <td>
          <a>
            <xsl:attribute name="href">
              <xsl:value-of select="dir/text()"/>/<xsl:value-of select="file/text()"/>
            </xsl:attribute>
            <!-- directory of the file of the stack location (add possible word breaks after each / character) -->
            <xsl:call-template name="replace-string">
              <xsl:with-param name="text" select="dir/text()"/>
              <xsl:with-param name="replace" select="'/'" />
              <xsl:with-param name="with" select="'/&#8203;'"/>
            </xsl:call-template>
            <!-- file of the stack location -->
            <xsl:if test="dir"><xsl:text>/&#8203;</xsl:text></xsl:if>
            <xsl:value-of select="file/text()"/>
          </a>
          <xsl:if test="line">:<xsl:value-of select="line/text()"/></xsl:if>
        </td>
        <!-- function name of the stack location (add possible word breaks after each :: character) -->
        <td>
          <xsl:call-template name="replace-string">
            <xsl:with-param name="text" select="fn/text()"/>
            <xsl:with-param name="replace" select="'::'" />
            <xsl:with-param name="with" select="'::&#8203;'"/>
          </xsl:call-template>
        </td>
        <!-- library of the stack location (add possible word breaks at each / character) -->
        <td>
          <xsl:call-template name="replace-string">
            <xsl:with-param name="text" select="obj/text()"/>
            <xsl:with-param name="replace" select="'/'" />
            <xsl:with-param name="with" select="'/&#8203;'"/>
          </xsl:call-template>
        </td>
      </tr>
    </xsl:if>
  </xsl:template>

  <!-- Helper function: replace text in string
  this must be called e.g. with
  <xsl:call-template name="replace-string">
    <xsl:with-param name="text" select="'aa::bb::cc'"/>
    <xsl:with-param name="replace" select="'::'" />
    <xsl:with-param name="with" select="','"/>
  </xsl:call-template>
  -->
  <xsl:template name="replace-string">
    <xsl:param name="text"/>
    <xsl:param name="replace"/>
    <xsl:param name="with"/>
    <xsl:choose>
      <xsl:when test="contains($text, $replace)">
        <xsl:value-of select="substring-before($text, $replace)"/>
        <xsl:value-of select="$with"/>
        <xsl:call-template name="replace-string">
          <xsl:with-param name="text" select="substring-after($text, $replace)"/>
          <xsl:with-param name="replace" select="$replace"/>
          <xsl:with-param name="with" select="$with"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$text"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

</xsl:stylesheet>
