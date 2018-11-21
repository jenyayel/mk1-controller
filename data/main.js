$(function () {

    function servoModel(stateLbl, increaseBtn, descreaseBtn, endpoint, low, high, jump) {
        var model = this;
        loadState(bindButtons);

        function bindState(state) {
            model.state = parseInt(state);

            normCurrent = model.state - low;
            normMax = high - low;
            var percentage = Math.round((normCurrent / normMax) * 100);
            stateLbl.text(percentage + '% (' + state + ')');
            stateLbl.width(percentage + '%');
        }

        function loadState(callback) {
            $.ajax({
                url: endpoint,
                type: 'GET',
                success: function (data) {
                    bindState(data);
                    if (callback) callback();
                },
                error: errorDetected
            });
        }

        function bindButtons() {
            function callApi(position) {
                if (position == model.state) return;
                $.ajax({
                    url: endpoint + '?position=' + position,
                    type: 'POST',
                    success: function () {
                        bindState(position);
                    },
                    error: errorDetected
                });
            }

            increaseBtn.click(function () {
                var newVal = model.state + jump > high ? high : model.state + jump;
                if (newVal == model.state) return;
                callApi(newVal);
            });
            descreaseBtn.click(function () {
                var newVal = model.state - jump < low ? low : model.state - jump;
                if (newVal == model.state) return;
                callApi(newVal);
            });
        }
    };

    var servos = [
        new servoModel($('#bl-front .progress-bar'), $('#bl-front button:first'), $('#bl-front button:last'), '/front', 115, 180, 5),
        new servoModel($('#bl-left .progress-bar'), $('#bl-left button:first'), $('#bl-left button:last'), '/left', 0, 170, 5),
        new servoModel($('#bl-right .progress-bar'), $('#bl-right button:first'), $('#bl-right button:last'), '/right', 50, 180, 5),
        new servoModel($('#bl-bottom .progress-bar'), $('#bl-bottom button:first'), $('#bl-bottom button:last'), '/bottom', 5, 180, 3)];

    var isInError = false;
    function errorDetected() {
        if (isInError === false) return;
        $('#error').show();
        isInError = true;
        $('button').prop('disabled', true);
    }
});