<?php

/**
 * @file
 * Drush commands for Isovera Tools.
 */

/**
 * Implements hook_drush_help().
 */
function isovera_drush_help($section) {
  switch ($section) {
    case 'drush:iso-git-commit-dump':
      return dt("Backup the site's database and create the filename with the following format:\n[current branch]-[date (YYYY-MM-DD)]-[7 character commit hash][-increment].sql.gz");
    case 'drush:iso-dl-commit':
      return dt("Download and commit a project");
    case 'meta:isovera:title':
      return dt("Isovera Tools commands");
    case 'meta:isovera:summary':
      return dt("Miscellaneous Isovera Tools");
  }
}

/**
 * Implements hook_drush_command().
 */
function isovera_drush_command() {
  $items = array();
  $items['iso-git-commit-dump'] = array(
    'description' => dt('Creates a sql-dump with filename that includes date, git branch and commit hash.'),
    'aliases' => array('isodump'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_DATABASE,
  );
  $items['iso-dl-commit'] = array(
    'description' => dt('Download and commit a Drupal project'),
    'aliases' => array('dlco'),
    'arguments' => array(
      'project' => 'A drupal.org project name (views, webform, etc.)',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_SITE,
    ),
  );

  return $items;
}

/**
 * Callback function for drush iso-git-commit-dump
 *
 */
function drush_isovera_iso_git_commit_dump() {

  // check if we can bootstrap
  $self = drush_sitealias_get_record('@self');
  if (empty($self)) {
    drush_die("I can't bootstrap from the current location.", 0);
  }

  /**
   * @TODO: Get server environment from settings.local.php configured variable
   * @TODO: Infer master branch or use tag for production code in detached HEAD state
   */
  $branch = exec('git rev-parse --abbrev-ref HEAD');
  $hash = exec('git log -n 1 --pretty=%h');
  $bak_dir = FALSE;
  $web_root = drush_invoke_process('@self', 'dd');
  $conf_bak_dir = variable_get('isovera_tools_backup_dir');

  $path = explode('/', $web_root['output']);
  array_pop($path);
  $bak_dir_default = implode('/', $path) . '/backup';
  $drush_bak_dir = '~/drush-backups';
  
  if(file_exists($conf_bak_dir)) {
    $bak_dir = $conf_bak_dir;
  }
  elseif(file_exists($bak_dir_default)) {
    $bak_dir = $bak_dir_default;
  }
  elseif(file_exists($drush_bak_dir)) {
    $bak_dir = $drush_bak_dir;
  }
  else {
    drush_die("No backup directories exist.", 0);
  }
  
  //  Convert slashes to dashes in branch name
  $branch = str_replace('/', '-', $branch);

  // Result directory path and filename
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

/**
 * Callback function for drush iso-git-commit-dump
 *
 */
function drush_tools_iso_dl_commit($project) {

  // check if we can bootstrap
  $self = drush_sitealias_get_record('@self');
  if (empty($self)) {
    drush_die("I can't bootstrap from the current location.", 0);
  }
  $projects = array($project);
  $options = array();

  drush_invoke_process('@self', 'dl', $projects, $options);
  drush_log("Code for git commits not yet added", 'ok');
}
