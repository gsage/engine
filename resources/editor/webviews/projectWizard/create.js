$(function(){
    engine.requestLocalization([
        "wizard.create.errors.path_empty",
        "wizard.create.errors.name_empty",
        "wizard.create.plugins_config.installed",
        "wizard.create.plugins_config.not_installed",
        "wizard.create.systems_config.not_set",
        "wizard.create.summary_config.window_manager",
        "wizard.create.summary_config.no_window_manager",
        "wizard.create.summary_config.input_manager",
        "wizard.create.summary_config.no_input_manager"
    ]).then(function(lm) {
        var step = 0;
        var steps = $(".step");
        var stepsCount = steps.length - 1;

        $("#plugins-config").accordion({
            exclusive: false
        });

        var preStepHandlers = {
            systems: function() {
                var processConfigs = function(systems, root, element, selected) {
                    var table = root.append("<table class=\"ui celled striped table\"><tbody id=\"" + element + "\"></tbody></table>");
                    if(!selected) {
                        selected = {};
                    }

                    var makeHandler = function(key) {
                        return function(value) {
                            if(value == "") {
                                delete selected[key];
                            } else {
                                selected[key] = value;
                            }
                        };
                    }

                    for(var key in systems) {
                        var tr = $("<tr/>", {});
                        tr.append($("<td>", {
                            "class": "right aligned collapsing",
                            html: key
                        }));
                        tr.append("<td><div class=\"ui selection dropdown\" id=\"" + key + "-select\"><div class=\"text\">" + lm["wizard.create.systems_config.not_set"] + "</div><i class=\"dropdown icon\"></i></div></td>");

                        $("#" + element).append(tr);
                        var values = [];
                        var anythingSelected = false;

                        for(var i = 0; i < systems[key].length; i++) {
                            var sysName = systems[key][i];
                            anythingSelected = anythingSelected || selected[key] == sysName;
                            values.push({
                                name: sysName,
                                value: sysName,
                                selected: selected[key] == sysName
                            });
                        }
                        values.splice(0, 0, {
                            name: lm["wizard.create.systems_config.not_set"],
                            value: "",
                            selected: !anythingSelected,
                        });

                        $("#" + key + "-select").dropdown({
                            values: values,
                            onChange: makeHandler(key)
                        });
                    }
                }
                if(settings.systems != null && Object.keys(settings.systems).length > 0) {
                    var systemsConfig = $("#systems-config");
                    systemsConfig.empty();
                    processConfigs(settings.systems, systemsConfig, "systems-config-body", settings.selectedSystems);
                    processConfigs(settings.additionalSystems, systemsConfig, "systems-config-body", settings.selectedSystems);
                }

                if(settings.managers != null && Object.keys(settings.managers).length > 0) {
                    var managersConfig = $("#managers-config");
                    managersConfig.empty();
                    processConfigs(settings.managers, managersConfig, "managers-config-body", settings.selectedManagers);
                }
            },

            summary: function() {
                // update summary data
                $("#summary-name").text(settings.projectName);
                $("#summary-path").text(settings.projectPath + "/" + settings.projectName);
                $("#summary-plugins").empty();
                for(var i = 0; i < settings.plugins.length; i++) {
                    var plugin = settings.plugins[i];
                    if(plugin.enabled) {
                        $("#summary-plugins").append("<tr><td>" + plugin.name + "</td></tr>");
                    }
                }
                $("#summary-systems").empty();
                for(var key in settings.selectedSystems) {
                    var system = settings.selectedSystems[key];
                    $("#summary-systems").append("<tr><td>" + key + "</td><td>" + system +"</td></tr>");
                }
                $("#summary-managers").empty();
                var windowManager = settings.selectedManagers.window
                var windowManagerInfo = windowManager ? lm["wizard.create.summary_config.window_manager"] + windowManager : lm["wizard.create.summary_config.no_window_manager"];

                var inputManager = settings.selectedManagers.input
                var inputManagerInfo = inputManager ? lm["wizard.create.summary_config.input_manager"] + inputManager : lm["wizard.create.summary_config.no_input_manager"];

                $("#summary-managers").append(
                    $("<div>", {
                        html: windowManagerInfo
                    })
                );
                $("#summary-managers").append(
                    $("<div>", {
                        html: inputManagerInfo
                    })
                );
            }
        };

        var postStepHandlers = {
            location: function() {
                settings.projectPath = $("#path").val();
                settings.projectName = $("#name").val();
                $("#location").submit();
                return !$("#location").hasClass("error");
            },

            plugins: function() {
                settings.additionalSystems = {}
                settings.plugins.forEach(function(plugin) {
                    if(plugin.enabled && "systems" in plugin.details) {
                        for(var key in plugin.details.systems) {
                            if(!(key in settings.additionalSystems)) {
                                settings.additionalSystems[key] = [];
                            }
                            settings.additionalSystems[key] = settings.additionalSystems[key].concat(plugin.details.systems[key]);
                        }
                    }
                }.bind(this));
                settings.managers = {}
                settings.plugins.forEach(function(plugin) {
                    if(plugin.enabled && "managers" in plugin.details) {
                        for(var key in plugin.details.managers) {
                            if(!(key in settings.managers)) {
                                settings.managers[key] = [];
                            }
                            settings.managers[key] = settings.managers[key].concat(plugin.details.managers[key]);
                        }
                    }
                }.bind(this));
                return true;
            },

            systems: function() {
                return true;
            },

            summary: function() {
                return true;
            }
        };

        function stepClick() {
            if($(this).hasClass("active")) {
                return;
            }

            setStep(steps.index(this));
        }

        steps.on("click", stepClick);

        function goBack() {
            if(step == 0) {
                engine.resetWizard();
                return;
            }

            setStep(step - 1);
        }

        function goNext() {
            if(step < stepsCount) {
                setStep(step + 1);
            } else {
                $("#complete").addClass("disabled")
                engine.createProject(settings);
            }
        }

        function setStep(value) {
            if(!postStepHandlers[$(steps[step]).attr("id")]()) {
                return;
            }

            var preStepHandler = preStepHandlers[$(steps[value]).attr("id")]
            if(preStepHandler) {
                preStepHandler();
            }

            steps.removeClass("active");
            var active = $(steps[value]);
            active.addClass("active");
            step = value;

            $(".tab").removeClass("active");
            $.tab("change tab", active.attr("id"));

            if(step > 0) {
                $("#cancel").hide();
                $("#back").show();
            } else {
                $("#cancel").show();
                $("#back").hide();
            }

            if(step == stepsCount) {
                $("#complete").show();
                $("#next").hide();
            } else {
                $("#complete").hide();
                $("#next").show();
            }
        };

        $("#cancel").on("click", goBack.bind(this));
        $("#back").on("click", goBack.bind(this));
        $("#next").on("click", goNext.bind(this));
        $("#complete").on("click", goNext.bind(this));
        $("#browse").on("click", function() {
            engine.pickFolder().then(function(path) {
                $("#path").val(path);
            }.bind(this));
        });

        $(".plugin-checkbox").checkbox({
            onChange: function() {
                var pluginName = this.id;
                var enabled = $("#" + pluginName).checkbox("is checked");
                for(var i = 0; i < settings.plugins.length; i++) {
                    if(settings.plugins[i].name == pluginName) {
                        settings.plugins[i].enabled = enabled;
                        break;
                    }
                }
                $("#" + pluginName + "-label").text(
                    enabled ? lm["wizard.create.plugins_config.installed"] : lm["wizard.create.plugins_config.not_installed"]
                );
            }
        });
        $('#location').form({
            on: 'blur',
            fields: {
                name: {
                    identifier  : 'name',
                    rules: [
                        {
                        type   : 'empty',
                        prompt : lm["wizard.create.errors.name_empty"]
                    }
                    ]
                },
                path: {
                    identifier  : 'path',
                    rules: [
                        {
                        type   : 'empty',
                        prompt : lm["wizard.create.errors.path_empty"]
                    }
                    ]
                }
            }
        });
    });
});

var phase = 0

window.goBack = function() {
    if(phase == 0) {
        engine.resetWizard();
    }
}

window.goNext = function() {
}
