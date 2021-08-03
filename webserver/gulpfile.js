var gulp = require('gulp');
var gzip = require('gulp-gzip');

var input_dist_folder = './dist/webserver/**/*.*';
var output_dist_folder = '../data';

gulp.task('compress', async function() {
  gulp.src([input_dist_folder])
      .pipe(gzip())
      .pipe(gulp.dest(output_dist_folder));
});
