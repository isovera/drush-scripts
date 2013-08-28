<?php

/**
 * Implements hook_drush_help().
 */
function isovera_tools_drush_help($command) {
  switch ($command) {
    case 'drush:git-commit-dump':
      return dt('Git commit dump');
  }
}

/**
 * Implements hook_drush_command().
 */
function isovera_tools_drush_command() {
  $items = array();
  $items['iso-git-commit-dump'] = array(
    'description' => dt('Creates a sql-dump with filename that includes date, git branch and commit hash.'),
    'aliases' => array('isodump'),
  );
  return $items;
}

/**
 * Callback function for drush refresh-learning-lens-styles
 *
 */
function drush_isovera_tools_iso_git_commit_dump() {

  // check if we can bootstrap
  $self = drush_sitealias_get_record('@self');
  if (empty($self)) {
    drush_die("I can't bootstrap from the current location.", 0);
  }

  /**
   * @TODO: Get server environment from settings.local.php configured variable
   * @TODO: Infer master branch or use tag for production code in detached HEAD state
   * @TODO: Replace / with - in branch names
   */
  $branch = exec('git rev-parse --abbrev-ref HEAD');
  $hash = exec('git log -n 1 --pretty=%h');
  $bak_dir = variable_get('isovera_tools_backup_dir');

  $result_file = $bak_dir . '/' . $branch . '_' . date('Y-m-d') . '_' . $hash;

  if(!file_exists("$result_file.sql.gz")) {
    $result_file .= '.sql';
  }
  else {
    $i = 0;
    while (file_exists("$result_file-$i.sql.gz")) {
      $i++;
    }
    $result_file .= "-$i.sql";
  }

  $options = array(
    'result-file' => $result_file,
    'structure-tables-key' => 'common',
  );

  drush_invoke_process('@self', 'sql-dump', array(), $options);
  exec("gzip $result_file");
  drush_log("Database dump gzipped to $result_file.gz", 'ok');
}