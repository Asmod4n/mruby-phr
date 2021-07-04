MRuby::Gem::Specification.new('mruby-phr') do |spec|
  spec.license = 'Apache-2'
  spec.author  = 'Hendrik Beskow'
  spec.summary = 'mruby gem for picohttpparser'

  picohttpparser_src = "#{spec.dir}/deps/picohttpparser"
  spec.cc.include_paths << "#{picohttpparser_src}"

  spec.objs += %W(
    #{picohttpparser_src}/picohttpparser.c
  ).map { |f| f.relative_path_from(dir).pathmap("#{build_dir}/%X#{spec.exts.object}" ) }
end
