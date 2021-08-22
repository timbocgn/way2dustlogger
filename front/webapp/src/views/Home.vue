<template>
  <v-container>
    <v-simple-table>
    
          <thead>
            <tr>
              <th colspan=3 class="text-left">Sensor Data</th>
            </tr>
          </thead>

          <tbody>   
            <template v-if="values.length != 0">
              <template v-for="(item,index) in values">

                  <tr :key="index">
                    <td>Sensor {{index+1}}</td>
                    <td class="grey--text">Value</td> 
                    <td class="grey--text">Unit</td> 
                  </tr>

                  <template v-if="loaded[index]==true">
                      <tr :key="index+1000">
                        <td class="grey--text">Big particles</td> 
                        <td>{{item.pm10.toFixed(2)}}</td>
                        <td class="grey--text"> ppm (10 um)</td> 
                      </tr>

                      <tr :key="index+2000">
                        <td class="grey--text">Medium particles</td> 
                        <td>{{item.pm2.toFixed(2)}}</td>
                        <td class="grey--text"> ppm (2.5 um)</td> 
                      </tr>

                      <tr :key="index+3000">
                        <td class="grey--text">Small particles</td> 
                        <td>{{item.pm1.toFixed(2)}}</td>
                        <td class="grey--text"> ppm (1 um)</td> 
                      </tr>                  
                  </template>
                  <template v-else>
                      <tr :key="index">
                        <td colspan=3 class="grey--text" :key="index">Loading...</td> 
                      </tr>
                </template>
              </template>
            </template>
            <template v-else>
              <tr>
                <td colspan=3 class="grey--text">Loading...</td> 
              </tr>
            </template>
          </tbody>
      </v-simple-table> 
  </v-container>
</template>



<script>
export default {
  data() {
    return {
      sensorcnt: null,
      values: [],
      loaded: [],
      timer: null
    };
  },
  
  // ---- cleanup the timer object

  destroyed()
  {
    clearInterval(this.timer);
  },

  // ---- define some custom methods

  methods: 
  {
    // ---- this one calls the AJAX functions and updates 

    updateData: function() 
    {
          this.$ajax
          .get("/api/v1/sensorcnt")
          .then(data => {

            this.sensorcnt  = data.data.cnt;

            var i;
            for (i = 0; i < this.sensorcnt; i++) 
            { 
                //console.log("Send request %d",i)
                var urlidx = i+1;
    
                this.$ajax
                          .get("/api/v1/air/" + urlidx.toString(),{ id: i})
                          .then(data => {
                                          this.values[data.config.id] = {pm1: data.data.pm1, pm2: data.data.pm2, pm10: data.data.pm10};
                                          this.loaded[data.config.id] = true;
                                          this.$forceUpdate();

                                          //console.log("Got Values %d",data.config.id);

                                          //console.log(data);
                                      }
                                )
                          .catch(error => {
                                          console.log(error);
                                        }
                                );
            }
          })
          .catch(error => {
            console.log(error);
          });    
    }
  }, 

  // ---- setup the timer

  mounted() 
  {
      clearInterval(this.timer);
      this.timer = setInterval(this.updateData , 1000);
  }
};
</script>
